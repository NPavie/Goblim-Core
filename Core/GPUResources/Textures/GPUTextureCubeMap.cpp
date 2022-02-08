#include "GPUTextureCubeMap.h"
#include <SOIL/SOIL.h>
#include "Engine/Base/Scene.h"
GPUTextureCubeMap::GPUTextureCubeMap(std::string name) : GPUTexture(name)
{
	m_Target = GL_TEXTURE_CUBE_MAP;
}

GPUTextureCubeMap::~GPUTextureCubeMap()
{
	glDeleteTextures(1,&m_TextureId);
	glDeleteSamplers(1,&m_SamplerId);
}

bool GPUTextureCubeMap::create(std::string file_right,std::string file_left,std::string file_top,std::string file_bottom,std::string file_back,std::string file_front)
{
	m_TextureId = SOIL_load_OGL_cubemap(
		file_right.c_str(),
		file_left.c_str(),
		file_top.c_str(),
		file_bottom.c_str(),
		file_back.c_str(),
		file_front.c_str(),
		SOIL_LOAD_AUTO,
		SOIL_CREATE_NEW_ID,
		SOIL_FLAG_NTSC_SAFE_RGB | SOIL_FLAG_COMPRESS_TO_DXT
		);
	
	
	if (m_TextureId == 0)
	{
		LOG(ERROR)  << "GPUTexture2D - SOIL loading error in cube Map " << this->m_Name << " : " << SOIL_last_result() << std::endl;
		return false;
	}

	setUpSampler(GL_CLAMP_TO_EDGE,GL_LINEAR);


	return (m_TextureId != 0);

}

void GPUTextureCubeMap::setUpSampler(GLint wrap,GLint filter)
{
	glBindSampler(m_Target,m_SamplerId);
	glSamplerParameteri(m_SamplerId , GL_TEXTURE_WRAP_R, wrap); 
	glSamplerParameteri(m_SamplerId , GL_TEXTURE_WRAP_S, wrap);  
	glSamplerParameteri(m_SamplerId , GL_TEXTURE_WRAP_T, wrap);  
	glSamplerParameteri(m_SamplerId , GL_TEXTURE_MIN_FILTER , filter);  
	glSamplerParameteri(m_SamplerId , GL_TEXTURE_MAG_FILTER , filter); 

}