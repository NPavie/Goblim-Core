#pragma once
#include <iostream>
#include <GL/glew.h>
#include "GPUTexture.h"
#include "GPUTexture2D.h"
#include <vector>
class DisplayResource;


/** 
*	Texture to handle opengl texture2DArray
*/
class GPUTexture2DArray : public GPUTexture
{
public:
	
	GPUTexture2DArray(std::string name);
	GPUTexture2DArray();

	//GL_COMPRESSED_RGBA_S3TC_DXT5_EXT for compressed format
	GPUTexture2DArray(std::string name,int width,int height,int depth,GLint internalFormat = GL_RGBA,GLint format = GL_RGBA,GLenum type = GL_FLOAT);
	~GPUTexture2DArray();

	bool create(int width,int height,int depth,GLint internalformat = GL_RGBA, GLint format = GL_RGBA,GLenum type = GL_FLOAT);
	
	/**
	*	@brief	Creation of a Texture2DArray from an array of Texture2D. All textures must have the same dimensions
	*/
	bool create(int width, int height, std::vector<GPUTexture2D*> textureArray );

	bool addLayer(unsigned char *data,int layer,int width,int height,int channels) ;

	/**
	*	@brief Load an image file in a layer of the Texture2DArray
	*/
	bool addLayer(std::string filename, int layer = -1,bool force=false, const glm::vec4 & default_Value = glm::vec4(1.0));

	int width();
	int height();

	void setUpSampler(GLint wrap, GLint minifyingFilter, GLint magnificationFilter);

private:
	int m_Width;
	int  m_Height;
	int m_Depth;
	int m_LayerCount;

};