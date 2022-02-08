#pragma once

#include "Engine/OpenGL/MaterialGL.h"
#include "Engine/OpenGL/Lighting/LightingModelGL.h"
#include "Materials/TextureGenerator/TextureGenerator.h"
#include "Engine/OpenGL/EngineGL.h"
#include "Engine/OpenGL/ModelGL.h"

#include "KernelList.h"
#include "Voxel.h"


class VolumetricKernelMaterial : public MaterialGL
{
public:
	VolumetricKernelMaterial(std::string name, KernelList* kernelsToRender, GPUTexture* depthTex, glm::vec2 windowSize);
	
	~VolumetricKernelMaterial();

	virtual void render(Node *o);
	virtual void update(Node *o, const int elapsedTime);

	GLint nbInstances;

private:
	ModelGL* cube;
	GPUmat4* CPU_MVP;
	GPUfloat* CPU_voxelSize;

	float fpsCounter;
	float frameCounter;


	TextureGenerator* surfaceData;
	GPUsampler* vp_smp_surfaceData;
	bool surfaceDataComputed;

	KernelList* kernelList;
	GPUfloat* f_Grid_height;
	GPUint* CPU_numFirstInstance;
	GPUvec2*	fp_v2_window_size;
	GPUTexture2D* depthTexture;

	GPUsampler* vp_smp_densityMaps;
	GPUsampler* vp_smp_scaleMaps;
	GPUsampler* vp_smp_distributionMaps;
	GPUsampler* fp_smp_models;
	GPUsampler* fp_smp_depth;
	
	
	GPUVoxelVector* voxelGrid;



};