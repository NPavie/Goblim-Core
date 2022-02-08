#ifndef _GOBLIM_DEPTHCOPY_EFFECT_
#define _GOBLIM_DEPTHCOPY_EFFECT_


#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>


class DepthCopy : public EffectGL
{
public:
	DepthCopy(std::string name);
	~DepthCopy();

	virtual void display(GPUTexture2D* depthTexture);


protected:
	GLProgram *fp;
	GLProgram *vp;

	glm::vec4 screenBox;

	GPUvec4* displaybox;
	GPUsampler* res;
};
#endif
