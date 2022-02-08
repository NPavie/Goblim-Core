#pragma once

#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>
#include "Engine/OpenGL/ModelGL.h"


class SpotProfile : public EffectGL
{
public:
	SpotProfile(std::string name, int spotID, bool isSpot = true, bool isDistribProfile=false);
	~SpotProfile();

	virtual void apply(GPUFBO *in, GPUFBO *out);



protected:
	
	GLProgram *vp;
	GLProgram* perPixelProgram;
	
	GPUsampler* FBO_in;
	
	GPUint* fp_spotID;
	GPUint* fp_isDistrib;
	GPUint* fp_isSpot;

};
