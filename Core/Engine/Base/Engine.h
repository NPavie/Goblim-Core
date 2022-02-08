/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */

#ifndef _ENGINE_H
#define _ENGINE_H

#include <map>
#include <string>
#include "Engine/Base/Scene.h"

class Engine
{
	public:
		Engine(){}; 
		~Engine(){};

	virtual bool init(){return true;};
	virtual void render(){};
	virtual void animate(const int elapsedTime){};

protected :
	Scene* scene;


};
#endif
