#ifndef _COLORMATERIAL_H
#define _COLORMATERIAL_H


#include "Engine/OpenGL/MaterialGL.h"
#include "Engine/OpenGL/LightingModelGL.h"
#include <memory.h>

class ColorMaterial : public MaterialGL
{
	public:
		ColorMaterial(std::string name, const glm::vec4 & c = glm::vec4(0.5,0.5,0.5,1.0));
		~ColorMaterial();
		void setColor(glm::vec4 & c);

		virtual void render(Node *o);
		virtual void update(Node* o,const int elapsedTime);
		GPUmat4* modelViewProj;
		GPUvec4* color;
		GPUmat4* modelM;
		GPUmat4* projM;

};

#endif