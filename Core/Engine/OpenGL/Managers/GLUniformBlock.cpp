/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */
#include <iostream>
#include "Engine/OpenGL/Managers/GLUniformBlock.h"
#include "Engine/OpenGL/v4/GLProgram.h"
#include "GPUResources/Buffers/GPUBuffer.h"

GLUniformBlock::GLUniformBlock(string name,GLProgram* program,int index,GLint size,bool shared):
	m_Name(name),m_Program(program),m_Index(index),m_Size(size),m_Shared(shared)
{
	m_BlockIndex = glGetUniformBlockIndex(m_Program->getProgram(),m_Name.c_str());
	ubo = NULL;
	if (!m_Shared)
	{
		ubo = new GPUBuffer(name);
		ubo->create(size);
		
	}
	m_BindingPoint = 0;
	shared = false;
}

GLUniformBlock::~GLUniformBlock()
{
	if (!m_Shared)
		delete ubo;

}

string GLUniformBlock::getName()
{
	return m_Name;
}

GLuint GLUniformBlock::getUniformBuffer()
{
	if (ubo == NULL)
		return -1;
	else
		return ubo->getBuffer();
}

void GLUniformBlock::bindBuffer()
{
	if (ubo != NULL)
		glBindBufferBase(GL_UNIFORM_BUFFER,m_BindingPoint,ubo->getBuffer());
}

void GLUniformBlock::bindBuffer(GLint bindingPoint)
{
	if (ubo != NULL)
		glBindBufferBase(GL_UNIFORM_BUFFER,bindingPoint,ubo->getBuffer());
}

void GLUniformBlock::attachBuffer(GPUBuffer *u)
{
	ubo = u;
}