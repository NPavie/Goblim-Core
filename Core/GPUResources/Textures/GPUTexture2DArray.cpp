#include "GPUTexture2DArray.h"
#include <SOIL/SOIL.h>
#include "Utils/ImageUtilities/ImageUtils.h"
#include "Engine/Base/Scene.h"
GPUTexture2DArray::GPUTexture2DArray():
GPUTexture("")
{
	m_Target = GL_TEXTURE_2D_ARRAY;
}
GPUTexture2DArray::GPUTexture2DArray(std::string name):
GPUTexture(name)
{
	m_Target = GL_TEXTURE_2D_ARRAY;
}
GPUTexture2DArray::GPUTexture2DArray(std::string name,int width,int height,int depth,GLint internalformat,GLint format ,GLenum type):
	GPUTexture(name)
{
	m_Target = GL_TEXTURE_2D_ARRAY;
	create(width,height,depth,internalformat,format,type);

}

void GPUTexture2DArray::setUpSampler(GLint wrap,GLint minifyingFilter,GLint magnificationFilter)
{
	glBindSampler(m_TextureId,m_SamplerId);
	glSamplerParameteri(m_SamplerId , GL_TEXTURE_WRAP_S, wrap);
	glSamplerParameteri(m_SamplerId , GL_TEXTURE_WRAP_T, wrap);
    glSamplerParameteri(m_SamplerId , GL_TEXTURE_MIN_FILTER , minifyingFilter);
    glSamplerParameteri(m_SamplerId , GL_TEXTURE_MAG_FILTER , magnificationFilter);
}


bool GPUTexture2DArray::create(int width,int height,int depth,GLint internalformat, GLint format,GLenum type)
{
	m_Width = width;
	m_Height = height;
	m_Depth = depth;
	glGenTextures(1,&m_TextureId);
	glBindTexture(m_Target,m_TextureId);
	int i = 0;
	glTexImage3D(m_Target,i,internalformat ,width, height,depth,0,format,type,NULL);
	glGenerateMipmap(m_Target);
	setUpSampler(GL_REPEAT,GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR);
	m_LayerCount = 0;
	return (true);
}


bool GPUTexture2DArray::addLayer(std::string filename, int layer,bool force, const glm::vec4 & defValue )
{
	if (layer == -1)
		layer = m_LayerCount;

	int width,height,channels;
	unsigned char * tex = SOIL_load_image(filename.c_str(),&width, &height, &channels,SOIL_LOAD_AUTO);
	if (tex == NULL && force)	// Forcing the creation of a texture with a default value
	{
		LOG(WARNING) << " GPUTexture2DArray - file " << filename << " not found. Using default value texture instead" << std::endl;
		width = m_Width;
		height = m_Height;
		channels = 4;
		int size = m_Width*m_Height;
		tex = new unsigned char[size*4];
		for(int i = 0;i < size;i++)
		{
			tex[4*i] = (unsigned char) (255*defValue[0]);
			tex[4*i+1] = (unsigned char) (255*defValue[1]);
			tex[4*i+2] = (unsigned char) (255*defValue[2]);
			tex[4*i+3] = (unsigned char) (255*defValue[3]);
		}
	}
	
	if (tex == NULL || layer >= m_Depth)
	{
		LOG(ERROR)  << " GPUTexture2DArray - SOIL loading error in file " << filename << " : " << SOIL_last_result() << std::endl;
		return(false);	
	}

	addLayer(tex,layer,width,height,channels) ;

	delete tex;

	return (true);
}
bool GPUTexture2DArray::addLayer(unsigned char *data,int layer,int width,int height,int channels)  
{
	if (layer < m_Depth)
	{
		glBindTexture(m_Target,m_TextureId);	
		loadLayer(data, width, height, channels,layer,m_Width,m_Height, NULL);	
		glGenerateMipmap(m_Target);
		m_LayerCount++;
	}
	return (layer < m_Depth);
}
GPUTexture2DArray::~GPUTexture2DArray()
{
	
}
