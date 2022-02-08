#ifndef SURFACIC_SPOT_NOISE_
#define SURFACIC_SPOT_NOISE_

#include "Engine/OpenGL/MaterialGL.h"
#include "GPUResources/Textures/GPUTexture2D.h"

#include <AntTweakBar/AntTweakBar.h>

#include <memory.h>

class SurfacicSpotNoise : public MaterialGL
{
	public:
		SurfacicSpotNoise(std::string name, int nbSpotPerAxis, int distribID, int noiseType = 0);
		~SurfacicSpotNoise();

	void addDataField(GPUTexture2D* dataTexture);

	virtual void render(Node *o);
	virtual void update(Node* o, const int elapsedTime);

	
protected:

	GPUmat4* modelViewMatrix;
	GPUmat4* modelMatrix;

	GPUTexture2D* tex_dataField;
	GPUsampler* smp_dataField;

	GPUint* fp_nbSpotPerAxis;
	GPUint* fp_noiseType;
	GPUint* fp_distribID;

	TwBar* noiseMenu;
	float scaling;
	GPUfloat* textureScalingFactor;



};

#endif