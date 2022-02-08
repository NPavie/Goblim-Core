#include "GPUResources/Textures/GPUTexture2D.h"
#include "Effects/DisplayResource/DisplayResource.h"
#include <SOIL/SOIL.h>
#include "Utils/ImageUtilities/ImageUtils.h"
#include "Utils/ImageReader/ImageReader.h"

#include "Utils/GLError.h"

GPUTexture2D::GPUTexture2D(std::string name,bool isResident):
GPUTexture(name)
{
	m_Target = GL_TEXTURE_2D;
	create(name);
	resident = isResident;
	if (resident)
		makeResident();

}
GPUTexture2D::GPUTexture2D():
GPUTexture("")
{
	m_Target = GL_TEXTURE_2D;

}
GPUTexture2D::GPUTexture2D(std::string name,int width,int height,GLint internalformat,GLint format ,GLenum type, bool isResident):
GPUTexture(name)
{
	resident = isResident;
	m_Target = GL_TEXTURE_2D;

	create(width,height,internalformat,format,type);

	if (resident)
		makeResident();
}

void GPUTexture2D::setUpSampler(GLint wrap,GLint minifyingFilter,GLint magnificationFilter)
{
	glBindSampler(m_TextureId,m_SamplerId);
	glSamplerParameteri(m_SamplerId , GL_TEXTURE_WRAP_S, wrap);
	glSamplerParameteri(m_SamplerId , GL_TEXTURE_WRAP_T, wrap);
    glSamplerParameteri(m_SamplerId , GL_TEXTURE_MIN_FILTER , minifyingFilter);
    glSamplerParameteri(m_SamplerId , GL_TEXTURE_MAG_FILTER , magnificationFilter);
}


bool GPUTexture2D::create(int width,int height,GLint internalformat, GLint format,GLenum type, bool isResident)
{
	resident = isResident;
	glGenTextures(1,&m_TextureId);
	glBindTexture(m_Target,m_TextureId);
	glTexImage2D(m_Target,0,internalformat ,width, height,0,format,type,NULL);
	setUpSampler(GL_REPEAT,GL_LINEAR,GL_LINEAR);
	m_Width = width;
	m_Height = height;
	if (resident)
		makeResident();
	
	return (true);
}



bool GPUTexture2D::create(std::string filename)
{

	//m_TextureId = load2DTexture(filename, SOIL_LOAD_AUTO);
	//notused: int width,height,channels;
	/*
	unsigned char * tex = SOIL_load_image(filename.c_str(),&width, &height, &channels,SOIL_LOAD_AUTO);
	glGenTextures(1,&m_TextureId);
	glBindTexture(m_Target,m_TextureId);
	glTexStorage2D(m_Target,10,GL_RGB8,width,height);
	glTexSubImage2D(m_Target,0,0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,tex);
	*/
	
	//m_TextureId = 0;

	m_TextureId = SOIL_load_OGL_texture
	(
		filename.c_str(),
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID, SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
	);
	
	/*
GFLImageReader *r = new GFLImageReader();

	m_TextureId = r->load2DTexture(filename);
*/






	
	glGenerateMipmap(m_Target);

	if (m_TextureId == 0)
	{
		LOG(ERROR) << " GPUTexture2D - SOIL loading error in file " << filename << " : " << SOIL_last_result() << std::endl;
		return false;
	}
	
	setUpSampler(GL_REPEAT,GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR);

	return (m_TextureId != 0);
}

bool GPUTexture2D::create(unsigned char *data, int width,int height, int channels)  
{
	
	m_TextureId = SOIL_create_OGL_texture(data,width,height,channels,SOIL_CREATE_NEW_ID,SOIL_FLAG_MIPMAPS | SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT);
	
	if (m_TextureId == 0)
	{
		LOG(ERROR) << " GPUTexture2D - loading from memory error : " << m_Name << " : " << SOIL_last_result() << std::endl;
		return false;
	}
	glGenerateMipmap(m_Target);
	setUpSampler(GL_REPEAT,GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR);
	m_Width = width;
	m_Height = height;
	return (m_TextureId != 0);
}

GPUTexture2D::~GPUTexture2D()
{
	
}
