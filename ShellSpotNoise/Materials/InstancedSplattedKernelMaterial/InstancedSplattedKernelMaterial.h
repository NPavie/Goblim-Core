#pragma once

#include "Engine/OpenGL/MaterialGL.h"
#include "Engine/OpenGL/Lighting/LightingModelGL.h"
#include "Materials/TextureGenerator/TextureGenerator.h"
#include "Engine/OpenGL/EngineGL.h"
#include "Engine/OpenGL/ModelGL.h"

#include "KernelList.h"
#include "Voxel.h"

class InstancedSplattedKernelMaterial : public MaterialGL
{
public:
	InstancedSplattedKernelMaterial(std::string name, KernelList* kernelsToRender, GPUFBO* accumulatorFBO, GPUTexture* depthTex);
	
	~InstancedSplattedKernelMaterial();

	virtual void render(Node *o);
	virtual void update(Node *o, const int elapsedTime);

	GLint nbInstances;
	bool useBestFragment;
	bool useSplatting;

	TextureGenerator* surfaceData;

private:
	ModelGL* quad;
	
	float fpsCounter;
	float frameCounter;

	bool surfaceDataComputed;
	
	// Voxels
	bool voxelListIsComputed;
	GPUVoxelVector* voxelList;

	KernelList* kernelList;

	GPUTexture2D* depthTexture;


	// Uniforms : pass 1
	GPUmat4* 	vp_CPU_MVP;
	GPUsampler* vp_smp_surfaceData;
	GPUfloat* 	vp_grid_height;
	GPUint* 	vp_CPU_numFirstInstance;
	GPUsampler* vp_smp_densityMaps;
	GPUsampler* vp_smp_scaleMaps;
	GPUsampler* vp_smp_distributionMaps;
	GPUmat4* 	fp_objectToScreen; 	// MVP
	GPUmat4* 	fp_objectToCamera; 	// MV
	GPUmat4* 	fp_objectToWorld;		// M
	GPUsampler* fp_smp_models;
	GPUsampler* fp_smp_depth;
	GPUvec2*	fp_cameraPlanes;
	

	GPUFBO* accumulator;
	
	// rajout d'une passe pour accumulation
	GLProgram* pass2_vp;
	GLProgram* pass2_fp;
	GLProgramPipeline* pass2;

	// Uniforms : pass 2
	GPUmat4* 	pass2_vp_CPU_MVP;
	GPUsampler* pass2_vp_smp_surfaceData;
	GPUfloat* 	pass2_vp_grid_height;
	GPUint* 	pass2_vp_CPU_numFirstInstance;
	GPUsampler* pass2_vp_smp_densityMaps;
	GPUsampler* pass2_vp_smp_scaleMaps;
	GPUsampler* pass2_vp_smp_distributionMaps;

	GPUmat4* 	pass2_fp_objectToScreen; 	// MVP
	GPUmat4* 	pass2_fp_objectToCamera; 	// MV
	GPUmat4* 	pass2_fp_objectToWorld;		// M
	GPUsampler* pass2_fp_smp_models;
	GPUsampler* pass2_fp_smp_depth;
	GPUvec2*	pass2_fp_cameraPlanes;


	
	
	// rajout d'une passe pour recomposition
	GLProgram* pass3_vp;
	GLProgram* pass3_fp;
	GLProgramPipeline* pass3;
	GPUsampler* pass3_firstFragmentSampler;
	GPUsampler* pass3_accumulatorSampler;
	GPUbool* pass3_fp_useBestFragment;
	GPUbool* pass3_fp_useSplatting;
	 


};