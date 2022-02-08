#include "GPUResources/Textures/GPUTexture.h"
#include "Effects/DisplayResource/DisplayResource.h"
#include <SOIL/SOIL.h>

GPUTexture::GPUTexture(std::string name):
Resource(name)
{
	HUD = new DisplayResource("HUD Texture");
	loadSampler();
	m_Handle = 0;
	resident = false;
}

GPUTexture::~GPUTexture()
{
	destroy();
	delete HUD;	
}


void GPUTexture::loadSampler()
{
	glGenSamplers(1 , &m_SamplerId); 	
	//glObjectLabel(GL_SAMPLER, m_SamplerId, -1, m_Name.c_str());

	//glGetObjectLabel(GL_SAMPLER, m_SamplerId, -1, m_Name.c_str());


}
bool GPUTexture::create()
{
	return true;
}
void GPUTexture::destroy()
{
	glDeleteTextures(1,&m_TextureId);
	glDeleteSamplers(1,&m_SamplerId);
}

void GPUTexture::bind(int channel)
{
	bound_on_channel = channel;
	GLuint target = GL_TEXTURE0 + channel;
	glActiveTexture(target);	
	glBindTexture(m_Target,m_TextureId);	
	glBindSampler(channel,m_SamplerId);
	
}
void GPUTexture::release()
{
	GLuint target = GL_TEXTURE0 + bound_on_channel;
	glActiveTexture(target);
	glBindTexture(m_Target,0);	
	glBindSampler(bound_on_channel,0);
}

GLuint GPUTexture::getId()
{
	return m_TextureId;
}
GLuint64 GPUTexture::getHandle()
{
	return m_Handle;
}
GLuint GPUTexture::getSampler()
{
	return m_SamplerId;
}
void GPUTexture::display(const glm::vec4 & box)
{
	bind(0);
	HUD->isUIntDepth->Set(false);
	HUD->display(box);
	release();
}

void GPUTexture::displayDepth(const glm::vec4 & box)
{
	bind(0);
	HUD->isUIntDepth->Set(true);
	HUD->display(box);
	release();
}

void GPUTexture::setUpSampler(GLint wrap,GLint filter)
{
	glBindSampler(m_Target,m_SamplerId);
	glSamplerParameteri(m_SamplerId , GL_TEXTURE_WRAP_S, wrap);  
    glSamplerParameteri(m_SamplerId , GL_TEXTURE_MIN_FILTER , filter);  
    glSamplerParameteri(m_SamplerId , GL_TEXTURE_MAG_FILTER , filter); 
}

void GPUTexture::makeResident()
{
	glBindTexture(m_Target, m_TextureId);
	m_Handle = glGetTextureHandleNV(m_TextureId);
	glMakeTextureHandleResidentNV(m_Handle);
	glBindTexture(m_Target, 0);
}
