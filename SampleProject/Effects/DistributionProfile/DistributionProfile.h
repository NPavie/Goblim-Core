#pragma once

#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>
#include "Engine/OpenGL/ModelGL.h"


class DistributionProfile : public EffectGL
{
public:
	DistributionProfile(std::string name, int distribID, int nbSpotPerAxis);
	~DistributionProfile();

	virtual void apply(GPUFBO *in, GPUFBO *out);

	void addDataField(GPUTexture2D * dataTexture);

protected:
	
	GLProgram *vp;
	GLProgram* perPixelProgram;
	
	GPUsampler* FBO_in;
	GPUsampler* smp_dataField;
	GPUTexture2D* tex_dataField;

	GPUint* fp_distribID;
	GPUint* fp_nbSpotPerAxis;

};
