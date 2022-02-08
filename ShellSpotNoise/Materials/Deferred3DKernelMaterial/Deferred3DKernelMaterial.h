#pragma once

#include "Engine/OpenGL/MaterialGL.h"
#include "Engine/OpenGL/Lighting/LightingModelGL.h"
#include "Materials/TextureGenerator/TextureGenerator.h"
#include "Engine/OpenGL/EngineGL.h"
#include "Engine/OpenGL/ModelGL.h"

#include "KernelList.h"

class Deferred3DKernelMaterial : public MaterialGL
{
public:
	Deferred3DKernelMaterial(std::string name, KernelList* kernelsToRender, glm::vec3 & metal_reflectance_roughness);

	~Deferred3DKernelMaterial();

	virtual void render(Node *o);
	virtual void update(Node *o, const int elapsedTime);

	GLint nbInstances;

private:
	ModelGL* cube;
	ModelGL* quad;

	float fpsCounter;
	float frameCounter;

	bool useBestFragment;
	bool useSplatting;

	TextureGenerator* surfaceData;

	bool surfaceDataComputed;

	KernelList* kernelList;

	GPUTexture2D* depthTexture;

	// Uniforms : pass 1
	GPUmat4* 	vp_objectToScreenSpace;
	GPUmat4*	vp_objectToCameraSpace;
	GPUmat4*	vp_lastMVP;
	GPUsampler* vp_smp_surfaceData;
	GPUfloat* 	vp_grid_height;
	GPUint* 	vp_CPU_numFirstInstance;
	GPUsampler* vp_smp_densityMaps;
	GPUsampler* vp_smp_scaleMaps;
	GPUsampler* vp_smp_distributionMaps;

	GPUmat4* 	fp_objectToScreen; 	// MVP
	GPUmat4* 	fp_objectToCamera; 	// MV
	GPUmat4* 	fp_objectToWorld;	// M
	GPUvec3*	fp_metalMask_Reflectance_Roughness;
	GPUsampler* fp_smp_models;
	GPUsampler* fp_smp_depth;




};