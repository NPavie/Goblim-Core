#ifndef TEXTURE_GENERATOR_
#define TEXTURE_GENERATOR_

#include "Engine/OpenGL/MaterialGL.h"

#include <memory.h>

class SampleMaterial : public MaterialGL
{
	public:
		SampleMaterial(std::string name);
		~SampleMaterial();

	virtual void render(Node *o);
	virtual void update(Node* o, const int elapsedTime);

	
protected:

	GPUmat4* modelViewMatrix;


};

#endif