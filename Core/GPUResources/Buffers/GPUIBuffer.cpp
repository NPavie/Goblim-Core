/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */
#include <iostream>
#include "GPUIBuffer.h"
#include "Engine/OpenGL/v4/GLProgram.h"

GPUIBuffer::GPUIBuffer(string name):
	Resource(name)
{
	
	
		
}
GPUIBuffer::~GPUIBuffer()
{
	destroy();
}
void GPUIBuffer::destroy()
{
	
	glDeleteBuffers(1,&m_Buffer);

}
void GPUIBuffer::create(GLenum target, GLsizeiptr size,const GLvoid * data, GLbitfield flags)
{
	m_Size = size;
	glGenBuffers(1, &m_Buffer);
	glBindBuffer(target,m_Buffer);
	glBufferStorage(target, size ,data, flags);

	m_BindingPoint = -1;

}

GLuint GPUIBuffer::getBuffer()
{
	return m_Buffer;
}

void GPUIBuffer::bind()
{
	glBindBufferBase(m_Target,m_BindingPoint,m_Buffer);
}
void GPUIBuffer::bind(int bindingPoint)
{
	glBindBufferBase(m_Target,bindingPoint,m_Buffer);
}
void GPUIBuffer::release()
{
	glBindBufferBase(m_Target,m_BindingPoint,0);
	
}