/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */

#ifndef _ENGINE_GL_H
#define _ENGINE_GL_H

#include <map>
#include <string>
#include "Engine/Base/Engine.h"
#include "Engine/Base/NodeCollectors/StandardCollector.h"
#include "Engine/OpenGL/LightingModelGL.h"
#include "GPUResources/Query/GPUQuery.h"

#include "Materials/BoundingBoxMaterial/BoundingBoxMaterial.h"

class EngineGL : public Engine
{
	public:
	EngineGL(int width, int height); 
	~EngineGL();

	virtual bool init();
	virtual void render();
	virtual void animate(const int elapsedTime);

	virtual void onWindowResize(int w,int h);
	
	double getFrameTime();

	int getWidth();
	int getHeight();

	void setWidth(int w); 
	void setHeight(int h);

	bool drawLights;
	bool drawBoundingBoxes;

protected:
	int w_Width;
	int w_Height;

	NodeCollector *allNodes;
	NodeCollector *renderedNodes;

	LightingModelGL *lightingModel;

	BoundingBoxMaterial *boundingBoxMat;
	 
	GPUQuery* timeQuery;
	

};
#endif
