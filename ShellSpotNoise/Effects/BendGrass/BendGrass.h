

#ifndef _BendGrassEffect_h
#define _BendGrassEffect_h

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
class BendGrass : public EffectGL
{
public:
	BendGrass(std::string name);
	~BendGrass();

	
	/**
	*	@brief apply the grass pattern over the colorFBO (depending on the target)
	*/
	void apply(TextureGenerator *surfaceData, const glm::vec4 & pos,float radius);
	
	void loadMenu();
	void reload();

	GPUvec2* mousePos;
	GPUfloat* timer;

	GPUFBO* bendTexture;
private:

	GLProgram *fp;
	GLProgram *vp;

	GPUsampler* groundData;
	GPUvec4* obj_pos;
	GPUfloat *radius;



};


#endif