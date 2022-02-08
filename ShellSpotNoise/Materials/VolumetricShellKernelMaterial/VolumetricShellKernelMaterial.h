#pragma once

#include "Engine/OpenGL/MaterialGL.h"
#include "Engine/OpenGL/Lighting/LightingModelGL.h"
#include "Materials/TextureGenerator/TextureGenerator.h"
#include "Engine/OpenGL/EngineGL.h"
#include "Engine/OpenGL/ModelGL.h"

#include "KernelList.h"
#include "Voxel.h"

class VolumetricShellKernelMaterial : public MaterialGL
{
public:
	VolumetricShellKernelMaterial(std::string name, KernelList* kernelsToRender, GPUFBO* accumulatorFBO, GPUTexture* depthTex);

	~VolumetricShellKernelMaterial();

	virtual void render(Node *o);
	virtual void update(Node *o, const int elapsedTime);


	void loadFromVoxelFile(const char* voxelFile);
	
	void precomputeVoxelList(Node* o, bool saveInFile = false, const char* fileName = NULL);

	GLint nbInstances;
	bool useBestFragment;

	TextureGenerator* surfaceData;

	

private:
	ModelGL* cube;
	ModelGL* quad;

	float fpsCounter;
	float frameCounter;
	
	bool surfaceDataComputed;

	KernelList* kernelList;

	GPUTexture2D* depthTexture;

	// Voxels
	bool voxelListIsComputed;
	GPUVoxelVector* voxelList;

	
	// Pass 1 : Evaluation de la modélisation dans la grille de voxel
	// Instancing des voxels
	// Pour chaque voxel, 
	// evaluation par déprojection des fragments dans les noyaux + pondération du résultat final par un OIT selon la position du voxel le long du rayon
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
	GPUmat4* 	fp_objectToWorld;	// M
	GPUsampler* fp_smp_models;
	GPUsampler* fp_smp_depth;


	GPUFBO* accumulator;

	// rajout d'une passe pour recomposition
	GLProgram* pass3_vp;
	GLProgram* pass3_fp;
	GLProgramPipeline* pass3;
	GPUsampler* pass3_firstFragmentSampler;
	GPUsampler* pass3_accumulatorSampler;
	GPUbool* pass3_fp_useBestFragment;



};