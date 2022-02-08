#ifndef TEXTURE_GENERATOR_
#define TEXTURE_GENERATOR_

#include "Engine/OpenGL/MaterialGL.h"
#include "GPUResources/FBO/GPUFBO.h"
#include <memory.h>

class TextureGenerator : public MaterialGL
{
	public:
	TextureGenerator(std::string name);
	~TextureGenerator();

	virtual void render(Node *o);

	
	void bindTextureArray(int channel);
	void releaseTextureArray();

	GPUFBO* getFBO();
	
protected:
	bool loadShaders();

	GPUmat3* modelViewMatrix;

	GPUFBO* textureStorage;


};

#endif