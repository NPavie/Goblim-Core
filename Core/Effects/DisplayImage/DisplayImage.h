#ifndef _GOBLIM_DISPLAYIMAGE_EFFECT_
#define _GOBLIM_DISPLAYIMAGE_EFFECT_


#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>


class DisplayImage : public EffectGL
{
	public:
		DisplayImage(std::string name);
		~DisplayImage();

		virtual void display(const glm::vec4 & box = glm::vec4(0.0,0.0,0.25,0.25));


	protected:
		GLProgram *fp;
		GLProgram *vp;

		GPUvec4* displaybox;
		GPUimage* res;
};
#endif
