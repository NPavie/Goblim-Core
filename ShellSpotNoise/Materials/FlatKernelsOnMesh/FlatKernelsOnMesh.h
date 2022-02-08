#pragma once

#include "Engine/OpenGL/MaterialGL.h"
#include "Engine/OpenGL/Lighting/LightingModelGL.h"
#include "Materials/TextureGenerator/TextureGenerator.h"
#include "Engine/OpenGL/EngineGL.h"
#include "Engine/OpenGL/ModelGL.h"

#include "Effects/ShellRayCompute/ShellRayCompute.h"
#include "KernelList.h"
#include "Voxel.h"

class FlatKernelsOnMesh : public MaterialGL
{
public:
	FlatKernelsOnMesh(std::string name, KernelList* kernelsToRender, GPUFBO* accumulatorFBO, GPUTexture* depthTex);
	
	~FlatKernelsOnMesh();

	virtual void render(Node *o);
	virtual void update(Node *o, const int elapsedTime);

	GLint nbInstances;
	bool useBestFragment;
	bool useSplatting;

	bool useInstacingOnly;

	TextureGenerator*	surfaceData;
	ShellRayCompute*	shellCompute;

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

	GPUFBO* rayDataFBO;

	// PASS 1
	GPUFBO* instancingWithDepth;
	// Uniforms : pass 1
	GPUmat4*	vp_objectToWorldSpace;	// Model matrix
	GPUfloat* 	vp_grid_height;
	GPUmat4* 	fp_objectToWorld;		// Model matrix

	GPUsampler* vp_smp_surfaceData;
	GPUsampler* vp_smp_densityMaps;
	GPUsampler* vp_smp_scaleMaps;
	GPUsampler* vp_smp_distributionMaps;
	GPUsampler* fp_smp_models;
	GPUsampler* fp_smp_depth;

	// Pour tests approfondi
	GPUsampler*	fp_smp_shellFront;
	GPUsampler*	fp_smp_shellBack;
	GPUsampler*	fp_smp_shellSurface;

	// PASS 2
	GPUFBO* splatting;
	// rajout d'une passe pour accumulation
	GLProgram* pass2_vp;
	GLProgram* pass2_fp;
	GLProgramPipeline* pass2;

	// Uniforms : pass 2
	GPUmat4* 	pass2_vp_objectToWorldSpace;
	GPUfloat* 	pass2_vp_grid_height;
	GPUmat4* 	pass2_fp_objectToWorld;		// M

	GPUsampler* pass2_vp_smp_surfaceData;
	GPUsampler* pass2_vp_smp_densityMaps;
	GPUsampler* pass2_vp_smp_scaleMaps;
	GPUsampler* pass2_vp_smp_distributionMaps;
	GPUsampler* pass2_fp_smp_models;
	GPUsampler* pass2_fp_smp_depth;
	GPUsampler*	pass2_fp_smp_shellFront;
	GPUsampler*	pass2_fp_smp_shellBack;
	GPUsampler*	pass2_fp_smp_shellSurface;



	// rajout d'une passe pour recomposition
	GLProgram* pass3_vp;
	GLProgram* pass3_fp;
	GLProgramPipeline* pass3;
	GPUsampler* pass3_firstFragmentSampler;
	GPUsampler* pass3_accumulatorSampler;
	GPUsampler* pass3_revealatorSampler;

	GPUbool* pass3_fp_useBestFragment;
	GPUbool* pass3_fp_useSplatting;
	 


};