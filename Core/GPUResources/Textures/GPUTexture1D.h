#pragma once
#include <iostream>
#include <GL/glew.h>
#include "GPUTexture.h"

class DisplayResource;

class GPUTexture1D : public GPUTexture
{
public:
	GPUTexture1D(std::string name);
	GPUTexture1D();
	GPUTexture1D(std::string name,int width,GLint internalFormat = GL_RGBA,GLint format = GL_RGBA,GLenum type = GL_FLOAT);
	~GPUTexture1D();

	
	
	bool create(int width,GLint internalformat = GL_RGBA, GLint format = GL_RGBA,GLenum type = GL_FLOAT);
	bool create(std::string filename);
	bool create(unsigned char *data, int width, int channels); 
	
	int width();
	int height();

	void setUpSampler(GLint wrap,GLint filter);

private:
	int m_Width;

};