#ifndef _SAMPLE_EFFECT_
#define _SAMPLE_EFFECT_

#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>
#include "Engine/OpenGL/ModelGL.h"


class SampleEffect : public EffectGL
{
public:
	SampleEffect(std::string name);
	~SampleEffect();

	virtual void apply(GPUFBO *in, GPUFBO *out);



protected:
	
	GLProgram *vp;
	GLProgram* perPixelProgram;
	
	GPUsampler* FBO_in;


};
#endif