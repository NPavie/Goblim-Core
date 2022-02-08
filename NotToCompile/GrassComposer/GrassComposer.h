

#ifndef _GrassEffect_h
#define _GrassEffect_h

#include "Engine/OpenGL/EffectGL.h"

#include "Materials/TextureGenerator/TextureGenerator.h"
#include "Engine/OpenGL/Lighting/LightingModelGL.h"
#include "Effects/ShellRayCompute/ShellRayCompute.h"
#include "Effects/DisplayLayer/DisplayLayer.h"

#include "Utils/MyTime.h"

#include "KernelList.h"
// Effet grassComposer : 
// Evalue le volumetric pattern
// Prérequis :
// - Position d'entrée du rayon dans l'espace texture
// - Position de sortie du rayon dans l'espace texture
// - Noyaux : modèles + paramètres (density maps etc.)


class EngineGL;
class GrassComposer : public EffectGL
{
public:
	GrassComposer(std::string name, KernelList* kernels, EngineGL* engineToUse);
	~GrassComposer();

	void setComposerFor(Node* target);
	
	/**
	*	@brief apply the grass pattern over the colorFBO (depending on the target)
	*/
	void apply(GPUFBO* colorFBO);
	
	void loadMenu();
	void reload();

	GPUvec2* mousePos;
	GPUfloat* timer;

private:

	GPUTexture2D* noiseTexture;
	GPUsampler* smp_noiseTexture;

	GLProgram *fp;
	GLProgram *vp;

	// Meilleure solution : laisser l'effet gérer lui même le FBO des données ?
	GPUFBO* rayDataFBO;
	
	ShellRayCompute* rayCompute;
	TextureGenerator* precomputedModel;
	bool precomputationDone;
	
	EngineGL*		engineToUse;
	Camera*		cameraToApplyOn;
	KernelList* kernels;
	LightingModelGL* lightingModel;
	
	GPUsampler*	fp_smp_surfaceData;
	GPUsampler* fp_smp_shellRayFront;
	GPUsampler* fp_smp_shellRayBack;
	GPUsampler* fp_smp_shellRaySurface;

	GPUsampler* fp_smp_colorFBO;
	GPUsampler* fp_smp_colorFBODepth;
	GPUsampler*	fp_smp_models;
	GPUsampler*	fp_smp_densityMaps;
	GPUsampler*	fp_smp_scaleMaps;
	GPUsampler* fp_smp_distributionMaps;
	
	Node*		target;

	GPUmat4* vp_MVP;
	
	// Matrices dans le fragment
	GPUmat4* fp_objectToScreen;
	GPUmat4* fp_objectToWorld;
	GPUmat4* fp_objectToCamera;
	GPUmat4* fp_worldToCamera;

	GPUvec4* fp_screenInfo;
	glm::vec4 screenInfo;

	GPUsampler* fp_smp_octree;

	GPUfloat* fp_gridHeight;
	
	GPUbool* fp_activeShadows;
	GPUbool* fp_activeAA;

	// Blowing in the wind
	GPUbool* fp_activeWind;
	GPUfloat* fp_windFactor;
	GPUfloat* fp_windSpeed;

	// DeadM4uB
	GPUbool* fp_activeMouse;
	GPUfloat* fp_mouseFactor;
	GPUfloat* fp_mouseRadius;

	GPUbool* fp_renderKernels;
	GPUbool* fp_renderGrid;

	TwBar* effectConfiguration;
	float gridHeight;

	ModelGL* shellAABBGeometry;

	DisplayLayer* displayData;

};


#endif