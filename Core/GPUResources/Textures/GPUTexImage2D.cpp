#include "GPUTexImage2D.h"

GPUTexImage2D::GPUTexImage2D(std::string name) : 
	GPUTexImage(name)
{
	m_Target = GL_TEXTURE_2D;
}

GPUTexImage2D::~GPUTexImage2D()
{
	
}

bool GPUTexImage2D::create(int w ,int h , int level , GLint format)
{
	m_Width = w;
	m_Height = h;
	m_Level = level;
	m_Format = format;
	
	glEnable(m_Target);
	glBindTexture(m_Target, m_ImageId);
	glTexStorage2D(m_Target, m_Level, m_Format,m_Width,m_Height);
	
	return true;
}

void GPUTexImage2D::bind(GLuint channel,GLenum accessmode,int level,GLenum format)
{
	bound_on_channel = channel;
	glBindImageTexture(channel,m_ImageId,level,false,0,accessmode,format == 0 ? m_Format:format);
}

void GPUTexImage2D::release()
{
	glBindImageTexture(bound_on_channel,0,0,GL_FALSE,0,GL_READ_ONLY,m_Format);
}

void GPUTexImage2D::clear(const glm::vec4 & color)
{
	glm::vec4 *val = new glm::vec4[m_Width*m_Height] ;
	for(int i = 0;i < m_Width*m_Height;i++)
		val[i] = glm::vec4(color);
	fill(val);
	delete val;
	
}
void GPUTexImage2D::fill(void* data,GLenum dataFormat,GLenum dataType)
{
	glTexSubImage2D(m_Target,0,0,0,m_Width,m_Height,dataFormat,dataType,data);
}

int GPUTexImage2D::width()
{
	return m_Width;
}
int GPUTexImage2D::height()
{
	return m_Height;
}
