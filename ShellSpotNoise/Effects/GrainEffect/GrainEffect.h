#ifndef GRAIN_EFFECT_H_
#define GRAIN_EFFECT_H_

#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>
#include "Engine/OpenGL/ModelGL.h"


class GrainEffect : public EffectGL
{
public:
	GrainEffect (std::string name);
	~GrainEffect ();

	void apply (GPUFBO* in, GPUFBO* out);

protected:

	GLProgram *vp;

	GLProgram *GrainFS;

	GPUsampler *Grain_Color;
	GPUvec2* resolution;
	GPUfloat* time;
};
#endif