#ifndef _GPUTEXIMAGE_
#define _GPUTEXIMAGE_

#include <iostream>
#include <GL/glew.h>
#include "Engine/Base/Resource.h"
#include <fstream>
#include <sstream>
#include "Engine/OpenGL/EffectGL.h"

#include "Effects/DisplayResource/DisplayResource.h"
#include "Effects/DisplayImage/DisplayImage.h"

	
// Image Texture class for Image Load - Store extension : Requires OpenGL 4.2 support
class GPUTexImage : public Resource
{
	public :

		GPUTexImage(std::string name);
		~GPUTexImage();

		// Create Image Texture Storage for Load and Store operation 
		virtual bool create();
		virtual void destroy();
		virtual void loadSampler();
		
			
		virtual void bind(GLuint channel = 0, GLenum accessmode = GL_READ_ONLY,int level=0,GLenum format=0);
		virtual void release();

		virtual void clear(const glm::vec4 & color = glm::vec4(0.0));

	GLuint getId();
		
	virtual void display(const glm::vec4 & box = glm::vec4(0.0,0.0,1.0,1.0));

	virtual void fill(void* data);
	
	protected :

	GLenum m_Target;
	GLenum m_Format;	
	GLuint m_ImageId;

	GLuint bound_on_channel;
	DisplayImage *HUD;
	GLuint m_SamplerId;
	

	std::string formatToString(int format = -1);
	int formatToType(int format = -1);
};

#endif