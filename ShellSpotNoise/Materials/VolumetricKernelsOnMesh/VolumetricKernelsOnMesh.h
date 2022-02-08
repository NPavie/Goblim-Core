#pragma once

#include "Engine/OpenGL/MaterialGL.h"
#include "Engine/OpenGL/Lighting/LightingModelGL.h"
#include "Materials/TextureGenerator/TextureGenerator.h"
#include "Engine/OpenGL/EngineGL.h"
#include "Engine/OpenGL/ModelGL.h"

#include "KernelList.h"

class VolumetricKernelsOnMesh : public MaterialGL
{
public:
	VolumetricKernelsOnMesh(std::string name, KernelList* kernelsToRender, GPUFBO* accumulatorFBO, GPUTexture* depthTex = NULL);

	~VolumetricKernelsOnMesh();

	virtual void render(Node *o);
	virtual void update(Node *o, const int elapsedTime);

	GLint nbInstances;

	bool useInstacingOnly;
	bool useBestFragment;
	bool useSplatting;

	int volKernelId;

private:
	ModelGL* cube;
	ModelGL* quad;

	float fpsCounter;
	float frameCounter;

	

	TextureGenerator* surfaceData;

	bool surfaceDataComputed;

	KernelList* kernelList;

	GPUTexture2D* depthTexture;

	// Uniforms : pass 1
	GPUmat4*	vp_objectToWorldSpace;	// Model matrix
	GPUfloat* 	vp_grid_height;
	GPUmat4* 	fp_objectToWorld;		// Model matrix
	GPUint*		fp_volKernelID;

	GPUsampler* vp_smp_surfaceData;
	GPUsampler* vp_smp_densityMaps;
	GPUsampler* vp_smp_scaleMaps;
	GPUsampler* vp_smp_distributionMaps;
	GPUsampler* vp_smp_colorMaps;
	
	GPUsampler* fp_smp_models;
	GPUsampler* fp_smp_depth;
	



	GPUFBO* accumulator;

	// rajout d'une passe pour accumulation
	GLProgram* pass2_vp;
	GLProgram* pass2_fp;
	GLProgramPipeline* pass2;

	// Uniforms : pass 2
	GPUmat4* 	pass2_vp_objectToWorldSpace;
	GPUfloat* 	pass2_vp_grid_height;
	GPUmat4* 	pass2_fp_objectToWorld;		// M
	GPUint*		pass2_fp_volKernelID;

	GPUsampler* pass2_vp_smp_surfaceData;
	GPUsampler* pass2_vp_smp_densityMaps;
	GPUsampler* pass2_vp_smp_scaleMaps;
	GPUsampler* pass2_vp_smp_distributionMaps;
	GPUsampler* pass2_vp_smp_colorMaps;
	GPUsampler* pass2_fp_smp_models;
	GPUsampler* pass2_fp_smp_depth;




	// rajout d'une passe pour recomposition
	GLProgram* pass3_vp;
	GLProgram* pass3_fp;
	GLProgramPipeline* pass3;
	GPUsampler* pass3_firstFragmentSampler;
	GPUsampler* pass3_accumulatorSampler;
	GPUbool* pass3_fp_useBestFragment;
	GPUbool* pass3_fp_useSplatting;



};