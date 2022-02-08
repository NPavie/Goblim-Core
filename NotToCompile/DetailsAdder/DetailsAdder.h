

#ifndef _DetailsAdder_h
#define _DetailsAdder_h


#include "Engine/OpenGL/MaterialGL.h"
#include "KernelList.h"
#include "Engine/OpenGL/Lighting/LightingModelGL.h"
#include "Engine/Base/Engine.h"

#include "Engine/Base/FacesOctree.h"
#include "Materials/TextureGenerator/TextureGenerator.h"
#include "Effects/ShellRayCompute/ShellRayCompute.h"

//#include "GPUResources/Textures/GPUTexture3D.h"

struct GPUVoxel
{
	glm::vec4 data[3];
	// min et max
	// distance et dir
	// closest UV
};

// Restructuration du code :
// sampling de la AABB sous la forme d'un ensemble de point

struct gridPoint
{
	glm::vec3 position;
	glm::vec2 UV;
	glm::vec3 surfacePoint;
	bool isInShell;
};

// la grille de voxel est alors composé de regroupements de 8 points

struct CPUVoxel
{
	glm::vec2 min_UV;
	glm::vec2 max_UV;
	glm::vec2 closest_UV;
	float min_distance;
	glm::vec3 dir_surface;

	gridPoint* sommets[8];

};

class DetailsAdder : public MaterialGL
{
public:
	DetailsAdder(std::string name, KernelList* kernels, Engine* engineToUse, Camera* cameraToApplyOn, LightingModelGL* lightList);
	~DetailsAdder();

	// Dans un premier temps pour le depth buffer, mais par la suite il sera beaucoup plus rapide de passer par le depth buffer pré chargé et de stoper l'évaluation comme ça.
	// Le materiau une fois rendu dans un FBO, doit passer par contre par un shader rapide de recomposition (blending entre le résultat et le background selon l'alpha de la couleur renvoyer
	void setBackgroundFBO(GPUFBO* color);
	void render(Node *o);
	void update(Node *o, const int elapsedTime){}

	void precomputeData(Node* target);
	void setVoxelGridSize(glm::ivec3 gridAxesSize);

	void loadMenu();
	void reload();
private:

	GPUFBO* colorFBO;
	GPUFBO* meshDataFBO;
	GPUFBO* pixelsData;

	bool precomputationDone;
	TextureGenerator* precomputedModel;
	ShellRayCompute*  perPixelComputing;

	Engine*		engineToUse;
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

	void computeOctree(AxisAlignedBoundingBox* shellAABB, GeometricModel* ObjectModel);




	// Test directement dans le materiau
	// grille de voxel
	
	std::vector<gridPoint*> *latter;

	glm::ivec3 voxelSubdiv;
	std::vector<CPUVoxel*> *voxelGrid;
	GPUBuffer* voxelBuffer;
	
	//GPUTexture3D* voxelUVTexture;
	//GPUTexture3D* voxelDistanceField;

	//GPUsampler* fp_smp_voxel_UV;
	//GPUsampler* fp_smp_voxel_distanceField;

};


#endif