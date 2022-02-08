
#include "PlanarNoiseMaterial.h"
//using namespace glm;


PlanarNoiseMaterial::PlanarNoiseMaterial(std::string name, KernelList* kernels, EngineGL* engineToUse, Camera* cameraToApplyOn, LightingModelGL* lightList) :
MaterialGL(name,"PlanarNoiseMaterial")
{
	
	
	// Pointer storage
	this->kernels = kernels;
	this->engineToUse = engineToUse;
	this->cameraToApplyOn = cameraToApplyOn;
	this->lightingModel = lightList;
	int bindingCounter = -1;

	// Surface data transfer
	precomputedModel = new TextureGenerator(name + "-ModelComputation");
	

	meshDataFBO = new GPUFBO(name + "-meshDataFBO");
	meshDataFBO->create(1024, 1024, 1, false, GL_RGBA16F, GL_TEXTURE_2D_ARRAY, 4);
	pixelsData = new GPUFBO(name + "-pixelsDataFBO");
	pixelsData->create(engineToUse->getWidth(), engineToUse->getHeight(), 2, false, GL_RGBA16F, GL_TEXTURE_2D, 1); // front face et backface des UV

	// Vertex shader uniforms
	vp_m44_Object_toScreen = vp->uniforms()->getGPUmat4("m44_Object_toScreen");
	vp_f_Grid_height = vp->uniforms()->getGPUfloat("f_Grid_height");


	// Fragment shader uniforms
	fp_v2_Screen_size = fp->uniforms()->getGPUvec2("v2_Screen_size");
	fp_v3_Object_cameraPosition = fp->uniforms()->getGPUvec3("v3_Object_cameraPosition");
	
	fp_m44_Object_toCamera = fp->uniforms()->getGPUmat4("m44_Object_toCamera");
	fp_m44_Object_toScreen = fp->uniforms()->getGPUmat4("m44_Object_toScreen");
	fp_m44_Object_toWorld = fp->uniforms()->getGPUmat4("m44_Object_toWorld");
	
	fp_v3_AABB_min = fp->uniforms()->getGPUvec3("v3_AABB_min");
	fp_v3_AABB_max = fp->uniforms()->getGPUvec3("v3_AABB_max");
	fp_f_Grid_height = fp->uniforms()->getGPUfloat("f_Grid_height");
	
	fp_smp_DepthTexture = fp->uniforms()->getGPUsampler("smp_DepthTexture");
	fp_smp_DepthTexture->Set(++bindingCounter);

	fp_smp_models			= fp->uniforms()->getGPUsampler("smp_models");
	fp_smp_models->Set(++bindingCounter);

	fp_smp_densityMaps		= fp->uniforms()->getGPUsampler("smp_densityMaps");
	fp_smp_scaleMaps		= fp->uniforms()->getGPUsampler("smp_scaleMaps");
	fp_smp_distributionMaps = fp->uniforms()->getGPUsampler("smp_distributionMaps");
	fp_smp_surfaceData		= fp->uniforms()->getGPUsampler("smp_surfaceData");
	fp_smp_voxel_UV			= fp->uniforms()->getGPUsampler("smp_voxel_UV");
	fp_smp_voxel_distanceField = fp->uniforms()->getGPUsampler("smp_voxel_distanceField");

	fp_renderKernels = fp->uniforms()->getGPUbool("renderKernels");
	fp_iv3_VoxelGrid_subdiv = fp->uniforms()->getGPUivec3("iv3_VoxelGrid_subdiv");

	fp_activeAA = fp->uniforms()->getGPUbool("activeAA");
	fp_activeShadows = fp->uniforms()->getGPUbool("activeShadows");
	fp_testFactor = fp->uniforms()->getGPUfloat("testFactor");
	fp_dMinFactor = fp->uniforms()->getGPUfloat("dMinFactor");
	fp_dMaxFactor = fp->uniforms()->getGPUfloat("dMaxFactor");

	fp_renderGrid = fp->uniforms()->getGPUbool("renderGrid");

	fp_modeSlicing = fp->uniforms()->getGPUbool("modeSlicing");

	cameraToApplyOn = Scene::getInstance()->camera();
	this->engineToUse = engineToUse;

	// Binding points
	
	fp_smp_densityMaps->Set(++bindingCounter);
	fp_smp_scaleMaps->Set(++bindingCounter);
	fp_smp_distributionMaps->Set(++bindingCounter);
	fp_smp_surfaceData->Set(++bindingCounter);
	fp_smp_voxel_UV->Set(++bindingCounter);
	fp_smp_voxel_distanceField->Set(++bindingCounter);

	this->kernels = kernels;
	lightingModel = lightList;

	precomputationDone = false;
	
}


void PlanarNoiseMaterial::render(Node* o)
{
	if (m_ProgramPipeline)
	{
		if (!precomputationDone) precomputeData(o);

		fp_modeSlicing->Set(kernels->modeSlicing);

		glm::vec3 camPos = Scene::getInstance()->camera()->convertPtTo(glm::vec3(0.0, 0.0, 0.0), o->frame());
		// Vertex stage
		vp_m44_Object_toScreen->Set(o->frame()->getTransformMatrix());
		vp_f_Grid_height->Set(kernels->getGridHeight());
		

		//fragment stage
		fp_v2_Screen_size->Set(glm::vec2(engineToUse->getWidth(), engineToUse->getHeight()));
		fp_v3_Object_cameraPosition->Set(camPos);
		
		fp_m44_Object_toCamera->Set(Scene::getInstance()->camera()->getModelViewMatrix(o->frame()));
		fp_m44_Object_toScreen->Set(o->frame()->getTransformMatrix());
		fp_m44_Object_toWorld->Set(o->frame()->getRootMatrix());
		fp_v3_AABB_min->Set(o->getModel()->getGeometricModel()->box->getMinValues());
		fp_v3_AABB_max->Set(o->getModel()->getGeometricModel()->box->getMaxValues());
		fp_testFactor->Set(kernels->testFactor);
		fp_dMinFactor->Set(kernels->dMinFactor);
		fp_dMaxFactor->Set(kernels->dMaxFactor);

		fp_f_Grid_height->Set(kernels->getGridHeight());

		fp_activeAA->Set(kernels->activeAA);
		fp_activeShadows->Set(kernels->activeShadows);
		fp_iv3_VoxelGrid_subdiv->Set(glm::ivec3(32,32,1));
		fp_renderGrid->Set(kernels->renderGrid);
		fp_renderKernels->Set(kernels->renderKernels);

		//colorFBO->bindColorTexture(fp_smp_DepthTexture->getValue(), 1);
		//kernels->bindModels(fp_smp_models->getValue());
		//kernels->bindDensityMaps(fp_smp_densityMaps->getValue());
		//kernels->bindScaleMaps(fp_smp_scaleMaps->getValue());
		//kernels->bindDistributionMaps(fp_smp_distributionMaps->getValue());
		meshDataFBO->bindColorTexture(fp_smp_surfaceData->getValue());

		// -- Draw call : apply material on node model
		m_ProgramPipeline->bind();
		o->drawGeometry(GL_TRIANGLES);
		m_ProgramPipeline->release();
		// */

		// Releasing the data
		meshDataFBO->releaseColorTexture();
		//kernels->releaseTextures();
		//colorFBO->releaseColorTexture();

		//meshDataFBO->display(glm::vec4(0, 0, 0.1, 0.1), 0);
		//pixelsData->display(glm::vec4(0, 0, 1, 1), 0);

		
	}
}


void PlanarNoiseMaterial::precomputeData(Node* target)
{
	
	meshDataFBO->enable();
	meshDataFBO->bindLayerToBuffer(0, 0);
	meshDataFBO->bindLayerToBuffer(1, 1);
	meshDataFBO->bindLayerToBuffer(2, 2);
	meshDataFBO->bindLayerToBuffer(3, 3);
	meshDataFBO->drawBuffers(4);
	precomputedModel->render(target);
	meshDataFBO->disable();
	precomputationDone = true;
}

void PlanarNoiseMaterial::setBackgroundFBO(GPUFBO* color)
{
	this->colorFBO = color;
}

