#ifndef _BOUNDINGBOXMATERIAL_H
#define _BOUNDINGBOXMATERIAL_H


#include "Engine/OpenGL/MaterialGL.h"
#include "Engine/OpenGL/Lighting/LightingModelGL.h"
#include <memory.h>

class BoundingBoxMaterial : public MaterialGL
{
	public:
		BoundingBoxMaterial(std::string name);
		~BoundingBoxMaterial();

		virtual void render(Node *o);
		virtual void update(Node* o,const int elapsedTime);
		GPUmat4* modelViewProj;
		GPUmat4* modelM;

};

#endif