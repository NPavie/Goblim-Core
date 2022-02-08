#ifndef DEFERRED_EFFECT 
#define DEFERRED_EFFECT 


#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>
#include "Engine/OpenGL/ModelGL.h"


class DeferredEffect : public EffectGL
{
public:
	DeferredEffect (std::string name, GPUTexture* test);
	~DeferredEffect ();

	void apply (GPUFBO *in, GPUFBO *out);



protected:

	GLProgram *vp;
	GLProgram* perPixelProgram;

	GPUsampler* FBO_in;
	//GPUsampler* FBO_test;
	GPUTexture* tex;

	GPUvec3 *dsCamPos;


};
#endif