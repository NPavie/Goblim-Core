#ifndef BLOOM_EFFECT_H_
#define BLOOM_EFFECT_H_

#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>
#include "Engine/OpenGL/ModelGL.h"


class BloomEffect : public EffectGL
{
public:
	BloomEffect (std::string name, float threshold, int blurSize);
	~BloomEffect ();

	void apply (GPUFBO* in, GPUFBO* out);

protected:

	GLProgram *vp;

	GLProgram *Bl_PRG_Bright_Computation;		// Bright computation Pass (Store every pixel above a threshold)
	GLProgram *Bl_PRG_Bright_HBlur;				// Bright horizontal blur Pass
	GLProgram *Bl_PRG_Bright_VBlur;				// Bright vertical blur Pass
	GLProgram *Bl_PRG_Bloom;					// Bloom Pass (Blend shaded image and blurred bright pixels)

	GPUFBO *Bl_FBO_Bright;
	GPUFBO *Bl_FBO_Bright_Blur[2];
	GPUFBO *Bl_FBO_Bloom;

	GPUsampler *Bl_Sampler_Bright_Computation_Color;
	GPUsampler *Bl_Sampler_Bright_HBlur_Bright;
	GPUsampler *Bl_Sampler_Bright_VBlur_Bright;
	GPUsampler *Bl_Sampler_Bloom_Color;
	GPUsampler *Bl_Sampler_Bloom_Blur;

	GPUfloat *blThreshold;
	GPUint *blHBlurSize;
	GPUfloat *blHBlurHStep;
	GPUint *blVBlurSize;
	GPUfloat *blVBlurVStep;
};
#endif