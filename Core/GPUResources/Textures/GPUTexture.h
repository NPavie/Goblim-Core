#pragma once
#include <iostream>
#include <GL/glew.h>
#include "Engine/Base/Resource.h"

class DisplayResource;

class GPUTexture : public Resource
{
public:
	GPUTexture(std::string name);
	~GPUTexture();

	
	virtual bool create();
	virtual void destroy();

	virtual void bind(int channel = 0);
	virtual void release();

	void makeResident();

	GLuint getId();
	GLuint64 getHandle();
	GLuint getSampler();

	virtual void display(const glm::vec4 & box = glm::vec4(0.0,0.0,1.0,1.0));

	virtual void displayDepth(const glm::vec4 & box = glm::vec4(0.0, 0.0, 1.0, 1.0));
	
	virtual void setUpSampler(GLint wrap,GLint filter);
protected:
	
	virtual void loadSampler();

	GLenum m_Target;
	GLuint bound_on_channel;
	DisplayResource *HUD;
	GLuint m_SamplerId;
	GLuint m_TextureId;
	GLuint64 m_Handle;
	GLint m_format;

	bool resident;

};