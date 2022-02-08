#include "GPUQuery.h"


GPUQuery::GPUQuery(string name, GLenum queryType) : Resource(name)
{
	m_queryBufferFront = 1;
	m_queryBufferBack = 0;
	this->m_queryType = queryType;
	m_updateResult = true;
	m_queryResult = NULL;
}

GPUQuery::~GPUQuery()
{
	destroy();
}

void GPUQuery::destroy()
{
	glDeleteQueries(1, this->m_queryId[m_queryBufferFront]);
	glDeleteQueries(1, this->m_queryId[m_queryBufferBack]);
}


void GPUQuery::create()
{
	glGenQueries(1, this->m_queryId[m_queryBufferFront]);
	glGenQueries(1, this->m_queryId[m_queryBufferBack]);
	// From lighthouse3D
	if (m_queryType == GL_TIME_ELAPSED) 
		glQueryCounter(m_queryId[m_queryBufferFront][0], GL_TIMESTAMP);
}

void GPUQuery::begin()
{
	glBeginQuery(m_queryType, m_queryId[m_queryBufferBack][0]);
}

void GPUQuery::end()
{
	glEndQuery(m_queryType);
	// Selon le type, mettre à jour le résultat
	switch (m_queryType)
	{
	case GL_TIME_ELAPSED:
		m_queryResult = new GLuint;
		glGetQueryObjectuiv(m_queryId[m_queryBufferFront][0], GL_QUERY_RESULT, (GLuint*)m_queryResult);
		break;
	default :
		m_queryResult = new GLint;
		glGetQueryObjectiv(m_queryId[m_queryBufferFront][0], GL_QUERY_RESULT, (GLint*)m_queryResult);
	}

	// swapping buffer for next acess
	if (m_queryBufferBack) {
		m_queryBufferBack = 0;
		m_queryBufferFront = 1;
	}
	else {
		m_queryBufferBack = 1;
		m_queryBufferFront = 0;
	}
}



GLint GPUQuery::getResultInt()
{
	return m_queryResult != NULL ? *((GLint*)m_queryResult) : 0;
}

GLint64 GPUQuery::getResultInt64()
{
	return  m_queryResult != NULL ? *((GLint64*)m_queryResult) : 0;
}

GLuint GPUQuery::getResultUInt()
{
	return  m_queryResult != NULL ? *((GLuint*)m_queryResult) : 0;
}

GLuint64 GPUQuery::getResultUInt64()
{
	return  m_queryResult != NULL ? *((GLuint64*)m_queryResult) : 0;
}