#ifndef _GOBLIM_DISPLAYRESOURCE_EFFECT_
#define _GOBLIM_DISPLAYRESOURCE_EFFECT_


#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>


class DisplayResource : public EffectGL
{
	public:
		DisplayResource(std::string name);
		~DisplayResource();

		virtual void display(const glm::vec4 & box = glm::vec4(0.0,0.0,0.25,0.25));

		GPUint* isUIntDepth;

	protected:
		GLProgram *fp;
		GLProgram *vp;

		GPUvec4* displaybox;

		GPUsampler* res;
};
#endif
