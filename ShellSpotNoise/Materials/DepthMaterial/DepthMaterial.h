#ifndef _DEPTHMATERIAL_H
#define _DEPTHMATERIAL_H


#include "Engine/OpenGL/MaterialGL.h"
#include "Engine/OpenGL/Lighting/LightingModelGL.h"
#include <memory.h>

class DepthMaterial : public MaterialGL
{
	public:
		DepthMaterial(std::string name);
		~DepthMaterial();

		virtual void render(Node *o);
		virtual void update(Node* o,const int elapsedTime);
		
		GPUmat4* CPU_MVP;
		GPUmat4* CPU_ModelView;
		

	protected:
		bool loadShaders();
};

#endif