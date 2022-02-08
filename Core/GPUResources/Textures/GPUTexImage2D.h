#ifndef _GPUTexImage2D_
#define _GPUTexImage2D_

#include <iostream>
#include <GL/glew.h>
#include "Engine/Base/Resource.h"
#include <fstream>
#include <sstream>
#include "GPUTexImage.h"
	
// Image Texture class for Image Load - Store extension : Requires OpenGL 4.2 support
class GPUTexImage2D : public GPUTexImage
{
	public :

		GPUTexImage2D(std::string name);
		~GPUTexImage2D();

		bool create(int w = 512,int h = 512, int level = 1,GLint format = GL_RGBA8);
			
		void bind(GLuint channel = 0, GLenum accessmode = GL_READ_ONLY,int level=0,GLenum format=0);
		void release();

		void clear(const glm::vec4 & color = glm::vec4(0.0));
		void fill(void* data,GLenum dataFormat = GL_RGBA,GLenum dataType = GL_FLOAT);

		int width();
		int height();



	private :

		
		int m_Width;
		int m_Height;
		int m_Level;
		
};

#endif