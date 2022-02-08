#pragma once

#include "Engine/OpenGL/MaterialGL.h"
#include "Engine/OpenGL/Lighting/LightingModelGL.h"
#include "Materials/TextureGenerator/TextureGenerator.h"
#include "Engine/OpenGL/EngineGL.h"
#include "Engine/OpenGL/ModelGL.h"

#include "KernelList.h"

class SSNMaterial : public MaterialGL
{
public:
	SSNMaterial(std::string name, KernelList* kernelsToRender, GPUFBO* accumulatorFBO, GPUTexture* depthTex);
	
	~SSNMaterial();

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

	KernelList* kernelList;

	GPUTexture2D* depthTexture;
	
	GPUFBO* accumulator;
	
	
	// Vertex Shaders
	GLProgram* vp_KernelBoundingQuad;	// For flat kernels
	GLProgram* vp_KernelBoundingCube;	// For volumetric kernels
	GLProgram* vp_Composition;			// For composition pass

	// Fragment Shaders
	GLProgram* fp_TexturedQuadRendering;	// For rendering of quads with depth test
	GLProgram* fp_TexturedQuadSplatting;	// For rendering of quads with weighted accumulation
	GLProgram* fp_CubeSlicing;				// For volumetric rendering of cubes
	GLProgram* fp_Composition;				// For composition

	// pipelines
	GLProgramPipeline* pass1;			// 
	GLProgramPipeline* pass2;
	GLProgramPipeline* pass3;


	// Uniforms : pass 1
	void initPass1(GLProgram* vertexShader, GLProgram* fragmentShader);
	GPUmat4* 	pass1_vu_CPU_MVP;
	GPUsampler* pass1_vu_smp_surfaceData;
	GPUfloat* 	pass1_vu_grid_height;
	GPUint* 	pass1_vu_CPU_numFirstInstance;
	GPUsampler* pass1_vu_smp_densityMaps;
	GPUsampler* pass1_vu_smp_scaleMaps;
	GPUsampler* pass1_vu_smp_distributionMaps;
	GPUmat4* 	pass1_fu_objectToScreen; 	// MVP
	GPUmat4* 	pass1_fu_objectToCamera; 	// MV
	GPUmat4* 	pass1_fu_objectToWorld;		// M
	GPUsampler* pass1_fu_smp_models;
	GPUsampler* pass1_fu_smp_depth;

	// Uniforms pass 2
	void initPass2(GLProgram* vertexShader, GLProgram* fragmentShader);
	GPUmat4* 	pass2_vu_CPU_MVP;
	GPUsampler* pass2_vu_smp_surfaceData;
	GPUfloat* 	pass2_vu_grid_height;
	GPUint* 	pass2_vu_CPU_numFirstInstance;
	GPUsampler* pass2_vu_smp_densityMaps;
	GPUsampler* pass2_vu_smp_scaleMaps;
	GPUsampler* pass2_vu_smp_distributionMaps;

	GPUmat4* 	pass2_fu_objectToScreen; 	// MVP
	GPUmat4* 	pass2_fu_objectToCamera; 	// MV
	GPUmat4* 	pass2_fu_objectToWorld;		// M
	GPUsampler* pass2_fu_smp_models;
	GPUsampler* pass2_fu_smp_depth;

	// Uniforms : pass 3
	void initPass3(GLProgram* vertexShader, GLProgram* fragmentShader);
	GPUsampler* pass3_firstFragmentSampler;
	GPUsampler* pass3_accumulatorSampler;
	GPUbool*	pass3_fu_useBestFragment;
	GPUbool*	pass3_fu_useSplatting;
	 


};