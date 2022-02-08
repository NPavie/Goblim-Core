#include "Materials/SplattedVolumetricKernelMaterial/SplattedVolumetricKernelMaterial.h"
#include "Engine/OpenGL/ModelGL.h"

SplattedVolumetricKernelMaterial::SplattedVolumetricKernelMaterial(std::string name, KernelList* kernelsToRender, GPUFBO* accumulatorFBO, GPUTexture* depthTex)
	: MaterialGL(name, "SplattedVolumetricKernelMaterial")
{
	vp_objectToScreenSpace = vp->uniforms()->getGPUmat4("objectToScreenSpace");
	vp_objectToCameraSpace = vp->uniforms()->getGPUmat4("objectToCameraSpace");
	vp_grid_height = vp->uniforms()->getGPUfloat("f_Grid_height");
	vp_CPU_numFirstInstance = vp->uniforms()->getGPUint("CPU_numFirstInstance");

	fp_objectToScreen = fp->uniforms()->getGPUmat4("objectToScreen");
	fp_objectToCamera = fp->uniforms()->getGPUmat4("objectToCamera");
	fp_objectToWorld = fp->uniforms()->getGPUmat4("objectToWorld");

	surfaceData = new TextureGenerator(name + "-surfaceData");

	quad = Scene::getInstance()->getModel<ModelGL>(ressourceObjPath + "Quad.obj");
	cube = Scene::getInstance()->getModel<ModelGL>(ressourceObjPath + "Cube.obj");
	kernelList = kernelsToRender;
	nbInstances = (GLint)(kernelList->at(0)->data[2].z * kernelList->at(0)->data[2].y * kernelList->at(0)->data[2].y);

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

	pass2_vp = NULL;
	pass2_fp = NULL;
	surfaceDataComputed = false;

	// Passes de EWA Splatting
	try{
		pass2 = new GLProgramPipeline(this->m_ClassName + "-SplattingPass");
		pass2_vp = new GLProgram(this->m_ClassName + "-Pass2", GL_VERTEX_SHADER);
		pass2_fp = new GLProgram(this->m_ClassName + "-Pass2", GL_FRAGMENT_SHADER);

		if (pass2_vp != NULL && pass2_vp->isValid()) 	pass2->useProgramStage(GL_VERTEX_SHADER_BIT, pass2_vp);
		if (pass2_fp != NULL && pass2_fp->isValid()) 	pass2->useProgramStage(GL_FRAGMENT_SHADER_BIT, pass2_fp);

		valid = pass2->link();

		// Pass 2 uniforms
		pass2_vp_objectToScreenSpace = pass2_vp->uniforms()->getGPUmat4("objectToScreenSpace");
		pass2_vp_grid_height = pass2_vp->uniforms()->getGPUfloat("f_Grid_height");
		pass2_vp_CPU_numFirstInstance = pass2_vp->uniforms()->getGPUint("CPU_numFirstInstance");
		pass2_fp_objectToScreen = pass2_fp->uniforms()->getGPUmat4("objectToScreen");
		pass2_fp_objectToCamera = pass2_fp->uniforms()->getGPUmat4("objectToCamera");
		pass2_fp_objectToWorld = pass2_fp->uniforms()->getGPUmat4("objectToWorld");

		// Pass 2 samplers
		pass2_fp_smp_models = pass2_fp->uniforms()->getGPUsampler("smp_models");
		pass2_fp_smp_depth = pass2_fp->uniforms()->getGPUsampler("smp_depthBuffer");
		pass2_vp_smp_densityMaps = pass2_vp->uniforms()->getGPUsampler("smp_densityMaps");
		pass2_vp_smp_scaleMaps = pass2_vp->uniforms()->getGPUsampler("smp_scaleMaps");
		pass2_vp_smp_distributionMaps = pass2_vp->uniforms()->getGPUsampler("smp_distributionMaps");
		pass2_vp_smp_surfaceData = pass2_vp->uniforms()->getGPUsampler("smp_surfaceData");

		// Copie des points de binding
		pass2_fp_smp_models->Set(fp_smp_models->getValue());
		pass2_fp_smp_depth->Set(fp_smp_depth->getValue());
		pass2_vp_smp_densityMaps->Set(vp_smp_densityMaps->getValue());
		pass2_vp_smp_scaleMaps->Set(vp_smp_scaleMaps->getValue());
		pass2_vp_smp_distributionMaps->Set(vp_smp_distributionMaps->getValue());
		pass2_vp_smp_surfaceData->Set(vp_smp_surfaceData->getValue());

		// Binding manuel des blocks sur un point de binding : au cas ou le binding= ne marcherait pas
		// Peut �tre le cas sur les cartes AMD (mais � re tester avec les derniers drivers)
		GLuint block_index = 0;
		block_index = glGetProgramResourceIndex(pass2_vp->getProgram(), GL_SHADER_STORAGE_BLOCK, "matricesBuffer");
		if (block_index != GL_INVALID_INDEX)
			glShaderStorageBlockBinding(pass2_vp->getProgram(), block_index, COMMON_SSBO_BINDINGS);


		block_index = glGetProgramResourceIndex(pass2_fp->getProgram(), GL_SHADER_STORAGE_BLOCK, "LightingBuffer");
		if (block_index != GL_INVALID_INDEX)
			glShaderStorageBlockBinding(pass2_fp->getProgram(), block_index, LIGHTING_SSBO_BINDING);


		block_index = glGetProgramResourceIndex(pass2_vp->getProgram(), GL_SHADER_STORAGE_BLOCK, "KernelBuffer");
		if (block_index != GL_INVALID_INDEX)
			glShaderStorageBlockBinding(pass2_vp->getProgram(), block_index, 3);


		block_index = glGetProgramResourceIndex(pass2_fp->getProgram(), GL_SHADER_STORAGE_BLOCK, "KernelBuffer");
		if (block_index != GL_INVALID_INDEX)
			glShaderStorageBlockBinding(pass2_fp->getProgram(), block_index, 3);

	}
	catch (const std::exception & e)
	{
		if (pass2_fp != NULL) delete pass2_fp;
		if (pass2_vp != NULL) delete pass2_vp;
		if (pass2 != NULL) delete pass2;

		throw logic_error(string("ERROR : Material ") + this->m_ClassName + string(" : \n") + e.what());
	}


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
		pass3_fp_useSplatting = pass3_fp->uniforms()->getGPUbool("useSplatting");


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

	useBestFragment = true;
	useSplatting = false;
}


SplattedVolumetricKernelMaterial::~SplattedVolumetricKernelMaterial()
{

}

void SplattedVolumetricKernelMaterial::render(Node *o)
{
	if (m_ProgramPipeline)
	{
		if (!surfaceDataComputed)
		{
			surfaceData->render(o);
			surfaceDataComputed = true;
		}




		int nbLigne = (kernelList->at(0)->data[2].y);
		nbInstances = (GLint)(kernelList->at(0)->data[2].z * nbLigne * nbLigne);

		// TODO : faire 1 material instancing volumetric standard, 1 material "Weighted Accumulator" et transformer ce materiau en recompositeur sp�cifique au mix
		// Passe 1 : rendu normal pour obtenir le fragment contributif le plus proche de la cam�ra
		accumulator->enable();
		if (useBestFragment)
		{
			accumulator->drawBuffer(0);
			glPushAttrib(GL_ALL_ATTRIB_BITS);
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//glDisable(GL_CULL_FACE);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);

			// VS uniforms :
			//CPU_numFirstInstance->Set(i * nbInstances);
			vp_CPU_numFirstInstance->Set(0);
			vp_grid_height->Set(kernelList->getGridHeight());
			vp_objectToScreenSpace->Set(o->frame()->getTransformMatrix());
			vp_objectToCameraSpace->Set(Scene::getInstance()->camera()->getModelViewMatrix(o->frame()));

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
			cube->drawInstancedGeometry(GL_TRIANGLES, nbInstances);
			m_ProgramPipeline->release();

			// Texture release
			surfaceData->releaseTextureArray();
			kernelList->releaseTextures();
			depthTexture->release();

			glPopAttrib();
		}

		if (useSplatting)
		{
			// Pass 2 : accumulateur (EWA Splatting)
			accumulator->drawBuffer(1);
			glClearColor(0, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_BLEND);
			// Accumulation des couleurs et alpha
			//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glBlendFunc(GL_ONE, GL_ONE);
			glBlendEquation(GL_FUNC_ADD);

			// VS uniforms :
			//CPU_numFirstInstance->Set(i * nbInstances);
			pass2_vp_CPU_numFirstInstance->Set(0);
			pass2_vp_grid_height->Set(kernelList->getGridHeight());
			pass2_vp_objectToScreenSpace->Set(o->frame()->getTransformMatrix());
			// FS uniforms
			pass2_fp_objectToScreen->Set(o->frame()->getTransformMatrix());
			pass2_fp_objectToCamera->Set(Scene::getInstance()->camera()->getModelViewMatrix(o->frame()));
			pass2_fp_objectToWorld->Set(o->frame()->getRootMatrix());

			// Texture binding
			kernelList->bindModels(fp_smp_models->getValue());
			depthTexture->bind(fp_smp_depth->getValue());
			kernelList->bindDensityMaps(vp_smp_densityMaps->getValue());
			kernelList->bindScaleMaps(vp_smp_scaleMaps->getValue());
			kernelList->bindDistributionMaps(vp_smp_distributionMaps->getValue());
			surfaceData->bindTextureArray(vp_smp_surfaceData->getValue());

			pass2->bind();
			cube->drawInstancedGeometry(GL_TRIANGLES, nbInstances);
			pass2->release();

			// Texture release
			surfaceData->releaseTextureArray();
			kernelList->releaseTextures();
			depthTexture->release();
		}
		

		accumulator->disable();

		// Compositing : a partir de l'accumulateur de fragment, on recompose le r�sultat
		pass3_fp_useBestFragment->Set(useBestFragment);
		pass3_fp_useSplatting->Set(useSplatting);
		accumulator->bindColorTexture(pass3_firstFragmentSampler->getValue(), 0);
		accumulator->bindColorTexture(pass3_accumulatorSampler->getValue(), 1);
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		// Accumulation des couleurs et alpha par dessus le rendu pr�c�dents
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		pass3->bind();
		quad->drawGeometry(GL_TRIANGLES);
		pass3->release();

		accumulator->releaseColorTexture();
		glDisable(GL_BLEND);
		glPopAttrib();


		//surfaceData->getFBO()->display(glm::vec4(0, 0, 0.1, 0.1), 0);
		//surfaceData->getFBO()->display(glm::vec4(0, 0.1, 0.1, 0.1), 1);
		//surfaceData->getFBO()->display(glm::vec4(0, 0.2, 0.1, 0.1), 2);
		//accumulator->display(glm::vec4(0, 0.3, 0.25, 0.25));
		//accumulator->display(glm::vec4(0, 0.0, 1.0, 1.0));




	}
}

void SplattedVolumetricKernelMaterial::update(Node *o, const int elapsedTime)
{
	if (Scene::getInstance()->camera()->needUpdate() || o->frame()->updateNeeded())
	{
		vp_objectToScreenSpace->Set(o->frame()->getTransformMatrix());
		fp_objectToScreen->Set(o->frame()->getTransformMatrix());
		fp_objectToCamera->Set(Scene::getInstance()->camera()->getModelViewMatrix(o->frame()));
		fp_objectToWorld->Set(o->frame()->getRootMatrix());

		pass2_vp_objectToScreenSpace->Set(o->frame()->getTransformMatrix());
		pass2_fp_objectToScreen->Set(o->frame()->getTransformMatrix());
		pass2_fp_objectToCamera->Set(Scene::getInstance()->camera()->getModelViewMatrix(o->frame()));
		pass2_fp_objectToWorld->Set(o->frame()->getRootMatrix());
	}
}