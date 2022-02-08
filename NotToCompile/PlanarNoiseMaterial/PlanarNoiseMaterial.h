

#ifndef _PlanarNoiseMaterial_h
#define _PlanarNoiseMaterial_h


#include "Engine/OpenGL/MaterialGL.h"

#include "Engine/OpenGL/Lighting/LightingModelGL.h"
#include "Engine/OpenGL/EngineGL.h"

#include "Effects/ShellRayCompute/ShellRayCompute.h"

#include "Materials/TextureGenerator/TextureGenerator.h"
#include "KernelList.h"

#include "Engine/Base/BoundingBox/BoundingBox.h"

class PlanarNoiseMaterial : public MaterialGL
{
public:
	PlanarNoiseMaterial(std::string name, KernelList* kernels, EngineGL* engineToUse, Camera* cameraToApplyOn, LightingModelGL* lightList);
	~PlanarNoiseMaterial();

	// Dans un premier temps pour le depth buffer, mais par la suite il sera beaucoup plus rapide de passer par le depth buffer pré chargé et de stoper l'évaluation comme ça.
	// Le materiau une fois rendu dans un FBO, doit passer par contre par un shader rapide de recomposition (blending entre le résultat et le background selon l'alpha de la couleur renvoyer
	void setBackgroundFBO(GPUFBO* color);
	void render(Node *o);
	void update(Node *o, const int elapsedTime){}

	void precomputeData(Node* target);

	void loadMenu();
	void reload();
private:

	GPUFBO* colorFBO;
	GPUFBO* meshDataFBO;
	GPUFBO* pixelsData;

	bool precomputationDone;
	TextureGenerator* precomputedModel;
	ShellRayCompute*  perPixelComputing;

	EngineGL*		engineToUse;
	Camera*		cameraToApplyOn;
	KernelList* kernels;
	LightingModelGL* lightingModel;

	// Vertex shader
	GPUmat4* vp_m44_Object_toScreen;
	GPUfloat* vp_f_Grid_height;
	
	// Fragment shader
	// Kernels sampler
	GPUsampler*	fp_smp_models;
	GPUsampler*	fp_smp_densityMaps;
	GPUsampler*	fp_smp_scaleMaps;
	GPUsampler* fp_smp_distributionMaps;
	
	// other sampler
	GPUsampler* fp_smp_DepthTexture;
	GPUsampler* fp_smp_surfaceData;

	GPUvec2*	fp_v2_Screen_size;
	GPUvec3*	fp_v3_Object_cameraPosition;
	GPUmat4*	fp_m44_Object_toCamera;
	GPUmat4*	fp_m44_Object_toScreen;
	GPUmat4*	fp_m44_Object_toWorld;

	GPUvec3*	fp_v3_AABB_min;
	GPUvec3*	fp_v3_AABB_max;
	GPUfloat*	fp_f_Grid_height;
	GPUivec3*	fp_iv3_VoxelGrid_subdiv;


	GPUsampler* fp_smp_colorFBO;

	//ModelViewProjectionMatrix
	GPUmat4* fp_objectToScreen;

	// Model matrix;
	GPUmat4* fp_objectToWorld;

	// MV matrix
	GPUmat4* fp_objectToCamera;

	// View matrix
	GPUmat4* fp_worldToCamera;

	GPUvec4* fp_screenInfo;
	glm::vec4 screenInfo;


	GPUfloat* fp_gridHeight;
	GPUfloat* gp_gridHeight;

	GPUfloat* fp_dMinFactor;
	GPUfloat* fp_dMaxFactor;

	GPUfloat* fp_testFactor;
	GPUbool* fp_renderKernels;
	GPUbool* fp_renderGrid;

	float gridHeight;
	
	GPUbool* fp_activeShadows;
	GPUbool* fp_activeAA;

	GPUbool* fp_modeSlicing;
	
	// Test directement dans le materiau
	// grille de voxel
	
	
	GPUsampler* fp_smp_voxel_UV;
	GPUsampler* fp_smp_voxel_distanceField;

};


#endif