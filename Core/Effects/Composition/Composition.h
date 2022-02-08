#ifndef _GOBLIM_COMPOSITION_EFFECT_
#define _GOBLIM_COMPOSITION_EFFECT_


#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>

class Composition : public EffectGL
{
	public:
		Composition(std::string name,int size = 512);
		~Composition();

		virtual void apply();



		


	protected:
		GLProgram *fp;
		GLProgram *vp;

		GPUfloat *bloomAmount;
		GPUsampler *LightScatterSampler,*colorSampler,*bloomSampler,*ssaoSampler,*depthSampler;
		GPUFBO *bloomFbo,*colorFbo,*lightFbo,*ssaoFbo;

};
#endif
