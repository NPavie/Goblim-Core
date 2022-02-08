#include "Materials/Deferred3DKernelMaterial/Deferred3DKernelMaterial.h"
#include "Engine/OpenGL/ModelGL.h"

Deferred3DKernelMaterial::Deferred3DKernelMaterial(std::string name, KernelList* kernelsToRender, glm::vec3 & metal_reflectance_roughness)
	: MaterialGL(name, "Deferred3DKernelMaterial")
{
	vp_objectToScreenSpace = vp->uniforms()->getGPUmat4("objectToScreenSpace");
	vp_objectToCameraSpace = vp->uniforms()->getGPUmat4("objectToCameraSpace");
	vp_grid_height = vp->uniforms()->getGPUfloat("f_Grid_height");
	vp_CPU_numFirstInstance = vp->uniforms()->getGPUint("CPU_numFirstInstance");
	vp_lastMVP = vp->uniforms()->getGPUmat4("lastMVP");

	fp_objectToScreen = fp->uniforms()->getGPUmat4("objectToScreen");
	fp_objectToCamera = fp->uniforms()->getGPUmat4("objectToCamera");
	fp_objectToWorld = fp->uniforms()->getGPUmat4("objectToWorld");
	fp_metalMask_Reflectance_Roughness = fp->uniforms()->getGPUvec3("metalMask_Reflectance_Roughness");
	fp_metalMask_Reflectance_Roughness->Set(metal_reflectance_roughness);

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


	// Textures binding point
	GLint bindingCounter = -1;
	fp_smp_models->Set(++bindingCounter);
	fp_smp_depth->Set(++bindingCounter);
	vp_smp_densityMaps->Set(++bindingCounter);
	vp_smp_scaleMaps->Set(++bindingCounter);
	vp_smp_distributionMaps->Set(++bindingCounter);
	vp_smp_surfaceData->Set(++bindingCounter);

	surfaceDataComputed = false;
	fpsCounter = 0;
	frameCounter = 0;

}


Deferred3DKernelMaterial::~Deferred3DKernelMaterial()
{

}

void Deferred3DKernelMaterial::render(Node *o)
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
				
		//glPushAttrib(GL_ALL_ATTRIB_BITS);
		//glClearColor(0, 0, 0, 0);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		////glDisable(GL_CULL_FACE);
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);

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
		//depthTexture->bind(fp_smp_depth->getValue());
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
		//depthTexture->release();

		//glPopAttrib();
		
	}
}

void Deferred3DKernelMaterial::update(Node *o, const int elapsedTime)
{
	vp_lastMVP->Set(vp_objectToScreenSpace->getValue());
	if (Scene::getInstance()->camera()->needUpdate() || o->frame()->updateNeeded())
	{
		vp_objectToScreenSpace->Set(o->frame()->getTransformMatrix());
		fp_objectToScreen->Set(o->frame()->getTransformMatrix());
		fp_objectToCamera->Set(Scene::getInstance()->camera()->getModelViewMatrix(o->frame()));
		fp_objectToWorld->Set(o->frame()->getRootMatrix());

	}
}