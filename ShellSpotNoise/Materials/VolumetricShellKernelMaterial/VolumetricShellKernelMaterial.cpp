#include "Materials/VolumetricShellKernelMaterial/VolumetricShellKernelMaterial.h"
#include "Engine/OpenGL/ModelGL.h"

VolumetricShellKernelMaterial::VolumetricShellKernelMaterial(std::string name, KernelList* kernelsToRender, GPUFBO* accumulatorFBO, GPUTexture* depthTex)
	: MaterialGL(name, "VolumetricShellKernelMaterial")
{
	vp_CPU_MVP = vp->uniforms()->getGPUmat4("CPU_MVP");
	vp_grid_height = vp->uniforms()->getGPUfloat("f_Grid_height");
	vp_CPU_numFirstInstance = vp->uniforms()->getGPUint("CPU_numFirstInstance");

	fp_objectToScreen = fp->uniforms()->getGPUmat4("objectToScreen");
	fp_objectToCamera = fp->uniforms()->getGPUmat4("objectToCamera");
	fp_objectToWorld = fp->uniforms()->getGPUmat4("objectToWorld");

	surfaceData = new TextureGenerator(name + "-surfaceData");

	cube = Scene::getInstance()->getModel<ModelGL>(ressourceObjPath + "Cube.obj");
	quad = Scene::getInstance()->getModel<ModelGL>(ressourceObjPath + "Quad.obj");
	kernelList = kernelsToRender;

	// Nombre de voxel à rendre
	nbInstances = 0;

	// Retrieve samplers
	fp_smp_models = fp->uniforms()->getGPUsampler("smp_models");
	fp_smp_depth = fp->uniforms()->getGPUsampler("smp_depthBuffer");
	vp_smp_densityMaps = vp->uniforms()->getGPUsampler("smp_densityMaps");
	vp_smp_scaleMaps = vp->uniforms()->getGPUsampler("smp_scaleMaps");
	vp_smp_distributionMaps = vp->uniforms()->getGPUsampler("smp_distributionMaps");
	vp_smp_surfaceData = vp->uniforms()->getGPUsampler("smp_surfaceData");

	depthTexture = (GPUTexture2D*)depthTex;

	// Textures binding point
	GLint bindingCounter = -1;
	fp_smp_models->Set(++bindingCounter);
	fp_smp_depth->Set(++bindingCounter);
	vp_smp_densityMaps->Set(++bindingCounter);
	vp_smp_scaleMaps->Set(++bindingCounter);
	vp_smp_distributionMaps->Set(++bindingCounter);
	vp_smp_surfaceData->Set(++bindingCounter);

	accumulator = accumulatorFBO;

	this->voxelList = new GPUVoxelVector();
	
	surfaceDataComputed = false;
		
	// Passes de Recomposition
	try{
		pass3 = new GLProgramPipeline(this->m_ClassName + "-RecompositionPass");
		pass3_vp = new GLProgram(this->m_ClassName + "-Pass3", GL_VERTEX_SHADER);
		pass3_fp = new GLProgram(this->m_ClassName + "-Pass3", GL_FRAGMENT_SHADER);

		if (pass3_vp != NULL && pass3_vp->isValid()) 	pass3->useProgramStage(GL_VERTEX_SHADER_BIT, pass3_vp);
		if (pass3_fp != NULL && pass3_fp->isValid()) 	pass3->useProgramStage(GL_FRAGMENT_SHADER_BIT, pass3_fp);

		valid = pass3->link();

		pass3_firstFragmentSampler = pass3_fp->uniforms()->getGPUsampler("firstFragment");
		pass3_firstFragmentSampler->Set(++bindingCounter);
		pass3_accumulatorSampler = pass3_fp->uniforms()->getGPUsampler("colorAccumulator");
		pass3_accumulatorSampler->Set(++bindingCounter);

		pass3_fp_useBestFragment = pass3_fp->uniforms()->getGPUbool("useBestFragment");


	}
	catch (const std::exception & e)
	{
		if (pass3_fp != NULL) delete pass3_fp;
		if (pass3_vp != NULL) delete pass3_vp;
		if (pass3 != NULL) delete pass3;

		throw logic_error(string("ERROR : Material ") + this->m_ClassName + string(" : \n") + e.what());
	}

	fpsCounter = 0;

	frameCounter = 0;

	useBestFragment = false;

	voxelListIsComputed = false;

}


VolumetricShellKernelMaterial::~VolumetricShellKernelMaterial()
{

}

void VolumetricShellKernelMaterial::render(Node *o)
{
	if (m_ProgramPipeline)
	{
		if (!surfaceDataComputed)
		{
			surfaceData->render(o);
			surfaceDataComputed = true;
		}
		
		if (!voxelListIsComputed)
		{
			this->voxelList = new GPUVoxelVector(o->getModel()->getGeometricModel(), 4, 8, 0.00f);
			this->voxelList->loadDataToGPU(4);
			voxelListIsComputed = true;
		}

				

		// Passe 1 : rendu normal pour obtenir le fragment contributif le plus proche de la caméra
		accumulator->enable();
		accumulator->drawBuffer(0);
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glDisable(GL_CULL_FACE);

		// VS uniforms :
		vp_CPU_numFirstInstance->Set(0);
		vp_grid_height->Set(kernelList->getGridHeight());
		vp_CPU_MVP->Set(o->frame()->getTransformMatrix());
		// FS uniforms
		fp_objectToScreen->Set(o->frame()->getTransformMatrix());
		fp_objectToCamera->Set(Scene::getInstance()->camera()->getModelViewMatrix(o->frame()));
		fp_objectToWorld->Set(o->frame()->getRootMatrix());
		
		// Texture binding
		kernelList->bindModels(fp_smp_models->getValue());
		depthTexture->bind(fp_smp_depth->getValue());
		kernelList->bindDensityMaps(vp_smp_densityMaps->getValue());
		kernelList->bindScaleMaps(vp_smp_scaleMaps->getValue());
		kernelList->bindDistributionMaps(vp_smp_distributionMaps->getValue());
		surfaceData->bindTextureArray(vp_smp_surfaceData->getValue());
		
		m_ProgramPipeline->bind();
		cube->drawInstancedGeometry(GL_TRIANGLES,this->voxelList->size());
		//cube->drawGeometry(GL_TRIANGLES);
		//o->drawGeometry(GL_TRIANGLES);
		m_ProgramPipeline->release();
		
		// Texture release
		surfaceData->releaseTextureArray();
		kernelList->releaseTextures();
		depthTexture->release();

		glPopAttrib();
		


		// Pass 2 : accumulateur (EWA Splatting)
		//accumulator->drawBuffer(1);
		//glClearColor(0, 0, 0, 0);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glDisable(GL_DEPTH_TEST);
		//glEnable(GL_BLEND);
		//// Accumulation des couleurs et alpha
		////glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//glBlendFunc(GL_ONE, GL_ONE);
		//glBlendEquation(GL_FUNC_ADD);

		//// VS uniforms :
		////CPU_numFirstInstance->Set(i * nbInstances);
		//pass2_vp_CPU_numFirstInstance->Set(0);
		//pass2_vp_grid_height->Set(kernelList->getGridHeight());
		//pass2_vp_CPU_MVP->Set(o->frame()->getTransformMatrix());
		//// FS uniforms
		//pass2_fp_objectToScreen->Set(o->frame()->getTransformMatrix());
		//pass2_fp_objectToCamera->Set(Scene::getInstance()->camera()->getModelViewMatrix(o->frame()));
		//pass2_fp_objectToWorld->Set(o->frame()->getRootMatrix());
		//
		//// Texture binding
		//kernelList->bindModels(fp_smp_models->getValue());
		//depthTexture->bind(fp_smp_depth->getValue());
		//kernelList->bindDensityMaps(vp_smp_densityMaps->getValue());
		//kernelList->bindScaleMaps(vp_smp_scaleMaps->getValue());
		//kernelList->bindDistributionMaps(vp_smp_distributionMaps->getValue());
		//surfaceData->bindTextureArray(vp_smp_surfaceData->getValue());
		//
		//pass2->bind();
		//quad->drawInstancedGeometry(GL_TRIANGLES, nbInstances);
		//pass2->release();
		//
		//// Texture release
		//surfaceData->releaseTextureArray();
		//kernelList->releaseTextures();
		//depthTexture->release();

		accumulator->disable();

		// Compositing : a partir de l'accumulateur de fragment, on recompose le résultat
		//pass3_fp_useBestFragment->Set(useBestFragment);
		//accumulator->bindColorTexture(pass3_firstFragmentSampler->getValue(), 0);
		//accumulator->bindColorTexture(pass3_accumulatorSampler->getValue(), 1);
		//glPushAttrib(GL_ALL_ATTRIB_BITS);
		//glDisable(GL_DEPTH_TEST);
		//glEnable(GL_BLEND);
		//// Accumulation des couleurs et alpha par dessus le rendu précédents
		//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		//
		//pass3->bind();
		//quad->drawGeometry(GL_TRIANGLES);
		//pass3->release();
		//
		//accumulator->releaseColorTexture();
		//glDisable(GL_BLEND);
		//glPopAttrib();



		//surfaceData->getFBO()->display(glm::vec4(0, 0, 0.1, 0.1), 0);
		//surfaceData->getFBO()->display(glm::vec4(0, 0.1, 0.1, 0.1), 1);
		//surfaceData->getFBO()->display(glm::vec4(0, 0.2, 0.1, 0.1), 2);
		accumulator->display(glm::vec4(0, 0.0, 0.25, 0.25));
		//accumulator->display(glm::vec4(0, 0.0, 0.1, 0.1));




	}
}

void VolumetricShellKernelMaterial::update(Node *o, const int elapsedTime)
{
	if (Scene::getInstance()->camera()->needUpdate() || o->frame()->updateNeeded())
	{
		vp_CPU_MVP->Set(o->frame()->getTransformMatrix());
		fp_objectToScreen->Set(o->frame()->getTransformMatrix());
		fp_objectToCamera->Set(Scene::getInstance()->camera()->getModelViewMatrix(o->frame()));
		fp_objectToWorld->Set(o->frame()->getRootMatrix());

	}
}


void VolumetricShellKernelMaterial::loadFromVoxelFile(const char* voxelFile)
{
	this->voxelList = new GPUVoxelVector(voxelFile);
	voxelListIsComputed = true;
}

void VolumetricShellKernelMaterial::precomputeVoxelList(Node* o, bool saveInFile, const char* fileName)
{
	this->voxelList = new GPUVoxelVector(o->getModel()->getGeometricModel());
	if (saveInFile && fileName != NULL)
	{
		this->voxelList->saveInFile(fileName);
	}
	voxelListIsComputed = true;
}