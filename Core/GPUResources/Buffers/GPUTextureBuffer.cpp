/*
 * GPUTextureBuffer.cpp
 *
 *  Created on: 14 déc. 2013
 *      Author: nicolas
 */

#ifndef GPUTEXTUREBUFFER_CPP_
#define GPUTEXTUREBUFFER_CPP_

#include "GPUTextureBuffer.h"

GPUTextureBuffer::GPUTextureBuffer() : GPUBuffer("")
{
	m_Type = GL_TEXTURE_BUFFER;
	m_Format = GL_NONE;
	bound_on_channel = -1;

	glGenBuffers(1, &m_Buffer);
}



GPUTextureBuffer::GPUTextureBuffer(std::string name)
: GPUBuffer("GPUTextureBuffer-"+name)
{
	m_Type = GL_TEXTURE_BUFFER;
	m_Format = GL_NONE;
	bound_on_channel = -1;

	glGenBuffers(1, &m_Buffer);
}

bool GPUTextureBuffer::create(int size, GLenum bufferUsage, GLint textureFormat )
{
	m_Size = size;
	
	glBindBuffer(GL_TEXTURE_BUFFER, m_Buffer);
	if (size != 0)	glBufferData(GL_TEXTURE_BUFFER, size, NULL, bufferUsage);
	
	bufferBound = false;
	
	m_Format = textureFormat;
	
	// Creating or updating texture information
	if (textureFormat != GL_NONE)	glGenTextures(1, &m_TextureId);
	else if( glIsTexture(m_TextureId) ) glDeleteTextures(1, &m_TextureId);

	return true;
}


void GPUTextureBuffer::transferBufferToTexture()
{
	//GLError::sendDebugNotification(string("Error in ") + string(m_Name) + string("::updateTexture() : the buffer is not set to be sampled as a texture\n"), m_Format == GL_NONE);
	if (m_Format != GL_NONE)
	{
		glBindTexture(GL_TEXTURE_BUFFER, m_TextureId);
		// Copy the buffer data to the texture
		glTexBuffer(GL_TEXTURE_BUFFER, m_Format, m_Buffer);
		//glBindTexture(GL_TEXTURE_BUFFER, 0);
	}
	
	
}

void GPUTextureBuffer::bindAsTexture(int channel)
{
	//GLError::sendDebugNotification(string("Error in ") + string(m_Name) + string("::bindAsTexture() : the buffer is not set to be sampled as a texture\n"), m_Format == GL_NONE);
	if (m_Format != GL_NONE)
	{
		bound_on_channel = channel;
		GLuint target = GL_TEXTURE0 + channel;
		glActiveTexture(target);
		glBindTexture(GL_TEXTURE_BUFFER, m_TextureId);
		//glBindSampler(channel, m_SamplerId);
	}
}

void GPUTextureBuffer::releaseTexture()
{
	//GLError::sendDebugNotification(string("Error in ") + string(m_Name) + string("::releaseTexture() : the buffer is not set to be sampled as a texture\n"), m_Format == GL_NONE);
	if (m_Format != GL_NONE)
	{
		GLuint target = GL_TEXTURE0 + bound_on_channel;
		glActiveTexture(target);
		glBindTexture(GL_TEXTURE_BUFFER, 0);
		//glBindSampler(bound_on_channel, 0);
	}
}

void GPUTextureBuffer::bind(int bindingPoint)
{
	glBindBuffer(GL_TEXTURE_BUFFER, m_Buffer);
	bufferBound = true;
}

void GPUTextureBuffer::release()
{
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	bufferBound = false;
}


void* GPUTextureBuffer::mapRange(int offset, int lengthOfRange, GLbitfield accessFlags)
{
	bind();
	// return the non typed pointer of the range
	return glMapBufferRange(GL_TEXTURE_BUFFER, offset, lengthOfRange, accessFlags);
}


/**
*	@brief unMap the Shader Storage Buffer being currently mapped
*/
void GPUTextureBuffer::unMap()
{
	glUnmapBuffer(GL_TEXTURE_BUFFER);

}

#endif /* GPUTEXTUREBUFFER_CPP_ */
