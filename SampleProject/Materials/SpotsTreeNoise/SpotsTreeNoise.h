#pragma once

#include "Engine/OpenGL/MaterialGL.h"
#include "Engine/OpenGL/Lighting/LightingModelGL.h"
#include "Engine/OpenGL/EngineGL.h"
#include "Engine/OpenGL/ModelGL.h"

#include "GPUSpotsTree.h"

#include "GPUResources/FBO/GPUFBO.h"


class SpotsTreeNoise : public MaterialGL
{
public:
	SpotsTreeNoise(std::string name, GPUSpotsTree* treeToRender, GPUFBO* accumulatorFBO, GPUTexture* depthTex);

	~SpotsTreeNoise();

	virtual void render(Node *o);
	virtual void update(Node *o, const int elapsedTime);

	GLint nbInstances;

private:
	
	float fpsCounter;
	float frameCounter;

	GPUSpotsTree* spotsTree;

	

	// uniforms
	GPUmat4*	vp_Model;



};