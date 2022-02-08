#pragma once

#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>
#include "Engine/OpenGL/ModelGL.h"


class SpotNoise : public EffectGL
{
public:
	SpotNoise(std::string name, int nbSpotPerAxis, int distribID, int noiseType = 0);
	~SpotNoise();

	void addDataField(GPUTexture2D* dataTexture);

	virtual void apply(GPUFBO *in, GPUFBO *out);



protected:
	
	GLProgram *vp;
	GLProgram* perPixelProgram;
	
	GPUsampler* FBO_in;

	GPUTexture2D* tex_dataField;
	GPUsampler* smp_dataField;

	GPUint* fp_nbSpotPerAxis;
	GPUint* fp_noiseType;
	GPUint* fp_distribID;


};
