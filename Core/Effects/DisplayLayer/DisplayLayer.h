#ifndef _GOBLIM_DISPLAYLAYER_EFFECT_
#define _GOBLIM_DISPLAYLAYER_EFFECT_


#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>

class DisplayLayer : public EffectGL
{
	public:
		DisplayLayer(std::string name);
		~DisplayLayer();

		virtual void display(const glm::vec4 & box = glm::vec4(0.0,0.0,0.25,0.25),int layer=0);


	protected:
		GLProgram *fp;
		GLProgram *vp;

		GPUvec4* displaybox;
		GPUsampler* res;
		GPUint*	layer_bound;
};
#endif
