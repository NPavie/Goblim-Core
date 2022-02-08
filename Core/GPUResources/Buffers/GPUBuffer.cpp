/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */
#include <iostream>
#include "GPUBuffer.h"
#include "Engine/OpenGL/v4/GLProgram.h"
//#include "GLError.h"

GPUBuffer::GPUBuffer(string name):
	Resource(name)
{
	maximumBinding = -1;
}
GPUBuffer::~GPUBuffer()
{
	destroy();
}
void GPUBuffer::destroy()
{
	m_isBound = false;
	m_BindingPoint = m_oldBindingPoint = - 1;
	glDeleteBuffers(1,&m_Buffer);
}
void GPUBuffer::create(GLint size,GLenum type, GLenum usage)
{
	m_Type = type;

	m_Size = size;
	glGenBuffers(1, &m_Buffer);
	
	if (size != 0)
	{
		glBindBuffer(type, m_Buffer);
		glBufferData(type, size, NULL, usage);
		glBindBuffer(type, 0);
	}
	
	m_isBound = false;
	m_BindingPoint = -1;
}


GLuint GPUBuffer::getBuffer()
{
	return m_Buffer;
}

GLenum GPUBuffer::getType()
{
	return m_Type;
}

void GPUBuffer::bind(int bindingPoint /* = -1 */)
{
	// Forced binding
	if (bindingPoint != -1) glBindBufferBase(m_Type, bindingPoint, m_Buffer);
	else
	{
		if (m_BindingPoint != -1)
			glBindBufferBase(m_Type, m_BindingPoint, m_Buffer);
		else
			glBindBuffer(m_Type, m_Buffer);
	}
	
	
	m_isBound = true;
	
}

void GPUBuffer::release(int fromBindingPoint /* = -1 */)
{
	// We consider the binding point to be correct at first binding so there's no need to retest the binding point
	if (m_isBound)
	{
		// In case of forced binding
		if (fromBindingPoint != -1)
		{
			glBindBufferBase(m_Type, fromBindingPoint, 0);
		}
		else
		{
			if (m_BindingPoint != -1)
				glBindBufferBase(m_Type, m_BindingPoint, 0);
			else
				glBindBuffer(m_Type, 0);

			if (m_oldBindingPoint != -1)
				m_BindingPoint = m_oldBindingPoint; // reposition buffer on its default binding
		}
		
	}
	m_isBound = false;
}

void GPUBuffer::setBindingPoint(int bindingPoint)
{
	//m_BindingPoint = bindingPoint;
	/*
	if (maximumBinding > 0)
	{
		if (bindingPoint < maximumBinding)
		{
			m_BindingPoint = bindingPoint;
			m_oldBindingPoint = bindingPoint; // keep a track of the assigned binding
		}
		else ;//GLError::sendDebugNotification(m_Name + "::Error: bad binding point : ", true);
	}
	
	else */
	m_BindingPoint = bindingPoint;
}

void GPUBuffer::Set(int offsetInByte, int sizeInByte, void* value)
{
	if (m_isBound) {
		if (m_Size > 0) 
			glBufferSubData(m_Type, offsetInByte, sizeInByte, value);
		else ;//GLError::sendDebugNotification(this->m_Name + ":: Error : buffer has not be allocated", true);
	}
	else
	{
		//GLError::sendDebugNotification(this->m_Name + ":: Error : buffer is not bound before setting data", true);
	}
}

void GPUBuffer::Get(int offsetInByte, int sizeInByte, void* value)
{
	if (m_isBound) {
		if (m_Size > 0) glGetBufferSubData(m_Type, offsetInByte, sizeInByte, value);
		//else ;//GLError::sendDebugNotification(this->m_Name + ":: Error : buffer has not be allocated", true);
	}
	else
	{
		//GLError::sendDebugNotification(this->m_Name + ":: Error : buffer is not bound on generic binding before setting data", true);
	}
}

bool GPUBuffer::isBound()
{
	return m_isBound;


}

void* GPUBuffer::mapRange(int offset, int lengthOfRange, GLbitfield accessFlags)
{
	if(!m_isBound) 	bind();
	// return the non typed pointer of the range
	return glMapBufferRange(m_Type, offset, lengthOfRange, accessFlags);
}

/**
*	@brief map the current buffer to host memory
*	@param accessFlag specify the access policy : must be GL_READ_ONLY or GL_WRITE_ONLY or GL_READ_WRITE
*	@return a non typed pointer to the first element of the buffer
*/
void* GPUBuffer::map(GLenum accessFlag)
{
	if (!m_isBound) bind();
	// return the non typed pointer of the whole storage
	return glMapBuffer(m_Type, accessFlag);
}

/**
*	@brief unMap the Shader Storage Buffer being currently mapped
*/
void GPUBuffer::unMap()
{
	glUnmapBuffer(m_Type);
	
	// Une barriere serait peut etre judicieuse à cet endroit
}

void GPUBuffer::bindToProgramStorageBlock(GLProgram* program, const char* bufferNameInGPU)
{
	GLuint block_index = glGetProgramResourceIndex(program->getProgram(), GL_SHADER_STORAGE_BLOCK, bufferNameInGPU);
	if (block_index != GL_INVALID_INDEX)
	{
		glShaderStorageBlockBinding(program->getProgram(), block_index, this->m_BindingPoint);
	}
	else
		std::cout << "/!\\ Warning :: bindBufferToProgramStorageBlock(" << program->getName() << "," << bufferNameInGPU << ")\n" << "/!\\ block not found in program\n";

}

void GPUBuffer::linkBindingPointToProgramStorageBlock(GLProgram* program, const char* bufferNameInGPU, int bindingPoint)
{
	GLuint block_index = glGetProgramResourceIndex(program->getProgram(), GL_SHADER_STORAGE_BLOCK, bufferNameInGPU);
	if (block_index != GL_INVALID_INDEX)
	{
		glShaderStorageBlockBinding(program->getProgram(), block_index, bindingPoint);
	}
	else
		std::cout << "/!\\ Warning :: bindBufferToProgramStorageBlock(" << program->getName() << "," << bufferNameInGPU << ")\n" << "/!\\ block not found in program\n";
}