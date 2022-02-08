#include "GPUShaderStorageBuffer.h"
#include "Utils/GLError.h"


GPUShaderStorageBuffer::~GPUShaderStorageBuffer()
{
	if (m_isBound) 
		release();
	destroy();
}

GPUShaderStorageBuffer::GPUShaderStorageBuffer(string name) :
GPUBuffer("GPUShaderStorageBuffer::"+name)
{
	
	glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &maximumBinding);

	m_Type = GL_SHADER_STORAGE_BUFFER;

}

void GPUShaderStorageBuffer::create(GLint size /*= 0*/, GLenum usage /*= GL_STREAM_DRAW*/) 
{
	this->GPUBuffer::create(size, m_Type, usage);

}


void GPUShaderStorageBuffer::setBindingPoint(int bindingPoint)
{
	m_BindingPoint = bindingPoint;
}


void* GPUShaderStorageBuffer::mapRange(int offset, int lengthOfRange, GLbitfield accessFlags)
{
	if(!m_isBound) 
		bind();
	// return the non typed pointer of the range
	return glMapBufferRange(m_Type, offset, lengthOfRange, accessFlags);
}

/**
*	@brief map the current buffer to host memory
*	@param accessFlag specify the access policy : must be GL_READ_ONLY or GL_WRITE_ONLY or GL_READ_WRITE
*	@return a non typed pointer to the first element of the buffer
*/
void* GPUShaderStorageBuffer::map(GLenum accessFlag)
{
	if (!m_isBound) 
		bind();
	// return the non typed pointer of the whole storage
	return glMapBuffer(m_Type, accessFlag);
}

/**
*	@brief unMap the Shader Storage Buffer being currently mapped
*/
void GPUShaderStorageBuffer::unMap()
{
	glUnmapBuffer(m_Type);
	
	// Une barriere serait peut etre judicieuse à cet endroit
}

