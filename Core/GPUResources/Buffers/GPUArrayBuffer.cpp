#include "GPUArrayBuffer.h"
#include "Utils/GLError.h"

GPUArrayBuffer::GPUArrayBuffer(string name) :
GPUBuffer("GPUShaderStorageBuffer::" + name)
{
	m_Type = GL_ARRAY_BUFFER;
	glGenBuffers(1, &m_Buffer);

}

void GPUArrayBuffer::create(GLint size /*= 0*/, GLenum usage /*= GL_STREAM_DRAW*/)
{
	m_Size = size;

	// Binding on generic state for allocation
	glBindBuffer(GL_ARRAY_BUFFER, m_Buffer);
	if (size != 0)	glBufferData(GL_ARRAY_BUFFER, size, NULL, usage);
	m_BindingPoint = -1;

	bufferBound = false;
}


void GPUArrayBuffer::bind(int bindingPoint /*= 0*/)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_Buffer);
	bufferBound = true;
	
}

void GPUArrayBuffer::release()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	bufferBound = false;
}


void* GPUArrayBuffer::mapRange(int offset, int lengthOfRange, GLbitfield accessFlags)
{
	bind();
	// return the non typed pointer of the range
	return glMapBufferRange(GL_ARRAY_BUFFER, offset, lengthOfRange, accessFlags);
}


/**
*	@brief unMap the Shader Storage Buffer being currently mapped
*/
void GPUArrayBuffer::unMap()
{
	glUnmapBuffer(GL_ARRAY_BUFFER);
	release();
	/*GLuint temp = m_BindingPoint;
	release();
	// If the buffer was previously bind at a specific position in the array of buffer, we rebind it
	if (temp > 0) bind(temp);*/
}


bool GPUArrayBuffer::isBound()
{
	return bufferBound;


}
