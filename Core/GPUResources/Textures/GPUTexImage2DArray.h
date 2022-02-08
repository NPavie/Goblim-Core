#ifndef _GPUTexImage2DArray_ARRAY_
#define _GPUTexImage2DArray_ARRAY_

#include <iostream>
#include <GL/glew.h>
#include "Engine/Base/Resource.h"
#include <fstream>
#include <sstream>
#include "GPUTexImage.h"
	
// Image Texture class for Image Load - Store extension : Requires OpenGL 4.2 support
class GPUTexImage2DArray : public GPUTexImage
{
	public :

		GPUTexImage2DArray(std::string name);
		~GPUTexImage2DArray();

		bool create(int w = 512,int h = 512,int d = 32, int level = 1,GLint format = GL_RGBA8);
			
		void bind(GLuint channel,GLenum accessmode = GL_READ_ONLY,int level=0,GLenum format=0);
		void bind(GLuint channel,int layer, GLenum accessmode = GL_READ_ONLY,int level=0,GLenum format=0);
		void release();

		void clear(const glm::vec4 & color = glm::vec4(0.0));
		void clearLayer(int layer, const glm::vec4 & color = glm::vec4(0.0));
		void fill(void* data,int layer = -1,GLenum internalFormat = GL_RGBA,GLenum dataType = GL_FLOAT);

		int width();
		int height();
		int depth();

		void display(int layer,const glm::vec4 & box = glm::vec4(0.0,0.0,1.0,1.0));


	private :

		
		int m_Width;
		int m_Height;
		int m_Depth;
		int m_Level;
		
};

#endif