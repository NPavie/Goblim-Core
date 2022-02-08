#include "GPUResources/Textures/GPUTexture1D.h"
#include "Effects/DisplayResource/DisplayResource.h"
#include <SOIL/SOIL.h>

GPUTexture1D::GPUTexture1D(std::string name):
GPUTexture(name)
{
	m_Target = GL_TEXTURE_1D;
	create(name);
	

}
GPUTexture1D::GPUTexture1D():
GPUTexture("")
{
	m_Target = GL_TEXTURE_1D;

}
GPUTexture1D::GPUTexture1D(std::string name,int width,GLint internalformat,GLint format ,GLenum type):
GPUTexture(name)
{
	m_Target = GL_TEXTURE_1D;
	create(width,internalformat,format,type);
}
void GPUTexture1D::setUpSampler(GLint wrap,GLint filter)
{
	glBindSampler(m_Target,m_SamplerId);
	glSamplerParameteri(m_SamplerId , GL_TEXTURE_WRAP_S, wrap);  
    glSamplerParameteri(m_SamplerId , GL_TEXTURE_MIN_FILTER , filter);  
    glSamplerParameteri(m_SamplerId , GL_TEXTURE_MAG_FILTER , filter); 
}


bool GPUTexture1D::create(int width,GLint internalformat, GLint format,GLenum type)
{
	glGenTextures(1,&m_TextureId);
	glBindTexture(m_Target,m_TextureId);
	glTexImage1D(m_Target,0,internalformat ,width, 0,format,type,NULL);	
	setUpSampler(GL_REPEAT,GL_LINEAR);
	m_Width = width;
	return (true);
}


bool GPUTexture1D::create(std::string filename)
{
	m_TextureId = SOIL_load_OGL_texture
	(
		filename.c_str(),
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	glGenerateMipmap(m_Target);
	if (m_TextureId == 0)
	{
		LOG(ERROR) << "ERROR : GPUTexture1D - SOIL loading error in file " << filename << " : " << SOIL_last_result() << std::endl;
		return false;
	}
	setUpSampler(GL_REPEAT,GL_LINEAR_MIPMAP_LINEAR);
	return (m_TextureId != 0);
}

bool GPUTexture1D::create(unsigned char *data, int width, int channels)  
{
	
	m_TextureId = SOIL_create_OGL_texture(data,width,1,channels,SOIL_CREATE_NEW_ID,SOIL_FLAG_MIPMAPS | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	
	if (m_TextureId == 0)
	{
		LOG(ERROR) << "ERROR : GPUTexture1D - loading from memory error : " << m_Name << " : " << SOIL_last_result() << std::endl;
		return false;
	}
	glGenerateMipmap(m_Target);
	setUpSampler(GL_REPEAT,GL_LINEAR_MIPMAP_LINEAR);
	m_Width = width;
	return (m_TextureId != 0);
}

GPUTexture1D::~GPUTexture1D()
{
	
}
