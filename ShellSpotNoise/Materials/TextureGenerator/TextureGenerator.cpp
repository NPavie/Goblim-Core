#include "Materials/TextureGenerator/TextureGenerator.h"
#include "Engine/Base/Node.h"



TextureGenerator::TextureGenerator(std::string name):
MaterialGL(name,"TextureGenerator")
{
	textureStorage = new GPUFBO(name + "-textureStorage");
	textureStorage->create(1024, 1024, 1, false, GL_RGBA16F, GL_TEXTURE_2D_ARRAY, 4);

}

TextureGenerator::~TextureGenerator()
{	

}


void TextureGenerator::render(Node *o)
{	
	if (m_ProgramPipeline)
	{
		//glPushAttrib(GL_ALL_ATTRIB_BITS);
		textureStorage->enable();
		textureStorage->bindLayerToBuffer(0, 0);
		textureStorage->bindLayerToBuffer(1, 1);
		textureStorage->bindLayerToBuffer(2, 2);
		textureStorage->bindLayerToBuffer(3, 3);
		textureStorage->drawBuffers(4);
		// Rendering in the FBO
			glClearColor(0.0,0.0,0.0,0.0);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			m_ProgramPipeline->bind();
			o->drawGeometry(GL_TRIANGLES);
			m_ProgramPipeline->release();
		// End
		textureStorage->disable();
		//glPopAttrib();
	}	
}

bool TextureGenerator::loadShaders(){
	return true;
}


void TextureGenerator::bindTextureArray(int channel)
{
	textureStorage->bindColorTexture(channel);
}
void TextureGenerator::releaseTextureArray()
{
	textureStorage->releaseColorTexture();
}

GPUFBO* TextureGenerator::getFBO()
{
	return textureStorage;
}