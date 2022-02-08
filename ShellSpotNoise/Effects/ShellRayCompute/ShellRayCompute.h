

#ifndef _ShellRayCompute_h
#define _ShellRayCompute_h

#include "Engine/OpenGL/EffectGL.h"
#include "Engine/OpenGL/Lighting/LightingModelGL.h"
#include "Effects/DisplayLayer/DisplayLayer.h"

// Effet ShellRayCompute : 
// Construit un rayon traversant une surcouche au dessus d'un maillage


class Engine;
class ShellRayCompute : public EffectGL
{
public:
	ShellRayCompute(std::string name);
	~ShellRayCompute();


	/**
	*	@brief apply the grass pattern to the selected node over the colorFBO
	*/
	void apply(Node* toNode, GPUFBO* colorFBO);

	void renderShellFront(Node* o);
	void renderShellBack(Node* o);
	void renderSurface(Node* o);

	void renderSurfaceBack(Node* o);
	
	void setShellHeight(float height);

	void loadMenu();
	//void reload();
private:
	GLProgram *vp_basic;
	GLProgram *vp_extruder;

	GLProgram *gp_borderCloser;

	GLProgram *fp_shellRayCompute;

	GLProgramPipeline* shellRenderer;
	GLProgramPipeline* surfaceRenderer;
	
	Camera*		currentCamera;
	
	GPUmat4* vp_basic_objectToScreen;			//ModelViewProjection matrix
	GPUfloat* vp_extruder_shellHeight;			// height of the grid to render

	GPUmat4* gp_borderCloser_objectToScreen;	//ModelViewProjection matrix

	GPUmat4* fp_shellRayCompute_objectToCamera;	// ModelView matrix


};


#endif