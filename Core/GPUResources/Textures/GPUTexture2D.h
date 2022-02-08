#pragma once
#include <iostream>
#include <GL/glew.h>
#include "GPUTexture.h"

class DisplayResource;

class GPUTexture2D : public GPUTexture
{
public:
	GPUTexture2D(std::string name,bool isResident = false);
	GPUTexture2D();
	GPUTexture2D(std::string name,int width,int height,GLint internalFormat = GL_RGBA,GLint format = GL_RGBA,GLenum type = GL_FLOAT,bool isResident = false);
	~GPUTexture2D();

	
	
	bool create(int width,int height,GLint internalformat = GL_RGBA, GLint format = GL_RGBA,GLenum type = GL_FLOAT, bool isResident = false);
	bool create(std::string filename);
	bool create(unsigned char *data, int width,int height, int channels); 
	
	int width();
	int height();
	void setUpSampler(GLint wrap,GLint minifyingFilter,GLint magnificationFilter);

private:
	int m_Width;
	int  m_Height;

};
