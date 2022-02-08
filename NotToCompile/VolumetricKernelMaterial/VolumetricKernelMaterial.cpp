#include "Materials/VolumetricKernelMaterial/VolumetricKernelMaterial.h"
#include "Engine/OpenGL/ModelGL.h"

VolumetricKernelMaterial::VolumetricKernelMaterial(std::string name, KernelList* kernelsToRender, GPUTexture* depthTex, glm::vec2 windowSize)
	: MaterialGL(name, "VolumetricKernelMaterial")
{
	CPU_MVP = vp->uniforms()->getGPUmat4("CPU_MVP");
	CPU_voxelSize = vp->uniforms()->getGPUfloat("CPU_voxelSize");
	
	f_Grid_height = vp->uniforms()->getGPUfloat("f_Grid_height");
	CPU_numFirstInstance = vp->uniforms()->getGPUint("CPU_numFirstInstance");
	
	fp_v2_window_size = fp->uniforms()->getGPUvec2("v2_window_size");
	fp_v2_window_size->Set(windowSize);

	surfaceData = new TextureGenerator(name + "-surfaceData");

	cube = Scene::getInstance()->getModel<ModelGL>(ressourceObjPath + "Cube.obj");

	kernelList = kernelsToRender;
	nbInstances = (GLint)(kernelList->at(0)->data[2].z * kernelList->at(0)->data[2].y * kernelList->at(0)->data[2].y);

	// Retrieve sampler
	// sampler du Fragment
	fp_smp_models	= fp->uniforms()->getGPUsampler("smp_models");
	fp_smp_depth = fp->uniforms()->getGPUsampler("smp_depth");

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

	

	surfaceDataComputed = false;
	
	

}


VolumetricKernelMaterial::~VolumetricKernelMaterial()
{

}

void VolumetricKernelMaterial::render(Node *o)
{
	if (m_ProgramPipeline)
	{
		if (!surfaceDataComputed)
		{
			surfaceData->render(o);
			// ça c'est pour la suite
			//voxelGrid = new GPUVoxelVector(o->getModel()->getGeometricModel(), 4, 6, 2.0f);
			//CPU_voxelSize->Set(voxelGrid->voxelHalfSize);
			surfaceDataComputed = true;
		}

		// Pipeline de rendu d'un octree de voxel en GPU
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		//glDisable(GL_DEPTH_TEST);
		//glEnable(GL_BLEND);
		//// Accumulation des couleurs et alpha
		//glBlendFunc(GL_SRC_COLOR, GL_ONE_MINUS_SRC_ALPHA);

		f_Grid_height->Set(kernelList->getGridHeight());
		CPU_MVP->Set(o->frame()->getTransformMatrix());
		
		kernelList->bindModels(fp_smp_models->getValue());
		depthTexture->bind(fp_smp_depth->getValue());

		kernelList->bindDensityMaps(vp_smp_densityMaps->getValue());
		kernelList->bindScaleMaps(vp_smp_scaleMaps->getValue());
		kernelList->bindDistributionMaps(vp_smp_distributionMaps->getValue());
		surfaceData->bindTextureArray(vp_smp_surfaceData->getValue());

		
		m_ProgramPipeline->bind();
		// premier test : instancing de cube
		cube->drawInstancedGeometry(GL_TRIANGLES, voxelGrid->size());
		//voxelGrid->getBoundingBoxModel()->drawGeometry();

		m_ProgramPipeline->release();

		surfaceData->releaseTextureArray();
		kernelList->releaseTextures();
		depthTexture->release();
			
		
		//glDisable(GL_BLEND);
		glPopAttrib();

		
		
		surfaceData->getFBO()->display(glm::vec4(0, 0, 0.1, 0.1), 0);
		surfaceData->getFBO()->display(glm::vec4(0, 0.1, 0.1, 0.1), 1);
		surfaceData->getFBO()->display(glm::vec4(0, 0.2, 0.1, 0.1), 2);
		//accumulator->display(glm::vec4(0, 0.3, 0.25, 0.25));
	
		
		

	}
}

void VolumetricKernelMaterial::update(Node *o, const int elapsedTime)
{
	if (Scene::getInstance()->camera()->needUpdate() || o->frame()->updateNeeded())
	{
		CPU_MVP->Set(o->frame()->getTransformMatrix());
	}
}