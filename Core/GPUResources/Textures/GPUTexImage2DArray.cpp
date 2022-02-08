#include "GPUTexImage2DArray.h"

GPUTexImage2DArray::GPUTexImage2DArray(std::string name) : 
	GPUTexImage(name)
{
	m_Target = GL_TEXTURE_2D_ARRAY;
}

GPUTexImage2DArray::~GPUTexImage2DArray()
{
	
}

bool GPUTexImage2DArray::create(int w ,int h , int d,int level , GLint format)
{
	m_Width = w;
	m_Height = h;
	m_Depth = d;
	m_Level = level;
	m_Format = format;
	
	glEnable(m_Target);
	glBindTexture(m_Target, m_ImageId);
	glTexStorage3D(m_Target, m_Level, m_Format,m_Width,m_Height,m_Depth);
	
	return true;
}

void GPUTexImage2DArray::bind(GLuint channel,GLenum accessmode,int level,GLenum format)
{
	bound_on_channel = channel;
	glBindImageTexture(channel,m_ImageId,level,GL_TRUE,0,accessmode,format == 0 ? m_Format:format);
}
void GPUTexImage2DArray::bind(GLuint channel,int layer,GLenum accessmode,int level,GLenum format)
{
	bound_on_channel = channel;
	glBindImageTexture(channel,m_ImageId,level,GL_FALSE,layer,accessmode,format == 0 ? m_Format:format);
}

void GPUTexImage2DArray::release()
{
	glBindImageTexture(bound_on_channel,0,0,GL_FALSE,0,GL_READ_ONLY,m_Format);
}

void GPUTexImage2DArray::clear(const glm::vec4 & color)
{
	glm::vec4 *val = new glm::vec4[m_Width*m_Height*m_Depth] ;
	for(int i = 0;i < m_Width*m_Height*m_Depth;i++)
		val[i] = glm::vec4(color);
	fill(val,-1);
	delete val;
}
void GPUTexImage2DArray::clearLayer(int layer,const glm::vec4 & color)
{
	glm::vec4 *val = new glm::vec4[m_Width*m_Height] ;
	for(int i = 0;i < m_Width*m_Height;i++)
		val[i] = glm::vec4(color);
	fill(val,layer);
	delete val;
	
}
void GPUTexImage2DArray::fill(void* data,int layer,GLenum internalFormat,GLenum dataType)
{
	if (layer == -1) // Fill whole 3D texture
		glTexSubImage3D(m_Target,0,0,0,0,m_Width,m_Height,m_Depth,internalFormat,dataType,data);
	else if (layer < m_Depth)
	{
		glTexSubImage3D(m_Target,0,0,0,layer,m_Width,m_Height,1,internalFormat,dataType,data);
	}
	else 
		LOG(WARNING) << " GPUTEXTUREIMAGE2DARAY : Trying to fill an inexisting layer of 3D texture" << std::endl;
}

int GPUTexImage2DArray::width()
{
	return m_Width;
}
int GPUTexImage2DArray::height()
{
	return m_Height;
}
int GPUTexImage2DArray::depth()
{
	return m_Depth;
}
void GPUTexImage2DArray::display(int layer, const glm::vec4 & box)
{
	bind(0,layer);
	HUD->display(box);
	release();
}