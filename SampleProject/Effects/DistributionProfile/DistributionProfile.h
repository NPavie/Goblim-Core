#pragma once

#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>
#include "Engine/OpenGL/ModelGL.h"


class DistributionProfile : public EffectGL
{
public:
	DistributionProfile(std::string name, int distribID);
	~DistributionProfile();

	virtual void apply(GPUFBO *in, GPUFBO *out);



protected:
	
	GLProgram *vp;
	GLProgram* perPixelProgram;
	
	GPUsampler* FBO_in;

	GPUint* fp_distribID;

};
