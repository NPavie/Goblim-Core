#include "GPUTransformFeedback.h"

#ifndef BUFFER_OFFSET(a)
	#define BUFFER_OFFSET(a) ((char*)NULL + (a))
#endif

GPUTransformFeedback::GPUTransformFeedback(std::string name)
	: Resource("TransformFeedback-"+name)
{

}
GPUTransformFeedback::~GPUTransformFeedback()
{
	destroy();
}
bool GPUTransformFeedback::create()
{
		
	m_TransformFeedback = -1;
	// Create feedback object
	glGenTransformFeedbacks(1,&m_TransformFeedback);
	glGenVertexArrays(1, &m_VertexArray);
	return ((bool)glIsTransformFeedback(m_TransformFeedback));

}

void GPUTransformFeedback::destroy()
{
	// destroy the openGL object
	glDeleteTransformFeedbacks(1, &m_TransformFeedback);
	glDeleteVertexArrays(1, &m_VertexArray);
}

void GPUTransformFeedback::setPipeline( const char* outputVariablesNames[], int nbVariables, int varyingsStage, GLenum bufferMode, GLProgram* vertexShader, GLProgram* geometryShader, GLProgram* tesselationControlShader /* = NULL */, GLProgram* tesselationEvalShader /* = NULL */ )
{

	m_transformPipeline = new GLProgramPipeline(this->m_Name+"-pipeline");
	m_transformPipeline->useProgramStage(GL_VERTEX_SHADER_BIT,vertexShader);

	if(geometryShader != NULL)
		m_transformPipeline->useProgramStage(GL_GEOMETRY_SHADER_BIT,geometryShader);

	if(tesselationControlShader != NULL)
		m_transformPipeline->useProgramStage(GL_TESS_CONTROL_SHADER_BIT,tesselationControlShader);

	if(tesselationEvalShader != NULL)
		m_transformPipeline->useProgramStage(GL_TESS_EVALUATION_SHADER_BIT,tesselationEvalShader);


	switch(varyingsStage)
	{

	case 3:
		if(tesselationEvalShader != NULL) glTransformFeedbackVaryings(tesselationEvalShader->getProgram(),nbVariables,outputVariablesNames,bufferMode);
		break;
	case 2:
		if(tesselationControlShader != NULL) glTransformFeedbackVaryings(tesselationControlShader->getProgram(),nbVariables,outputVariablesNames,bufferMode);
		break;
	case 1:
		if(geometryShader != NULL) glTransformFeedbackVaryings(geometryShader->getProgram(),nbVariables,outputVariablesNames,bufferMode);
		break;

	case 0:
	default:
		glTransformFeedbackVaryings(vertexShader->getProgram(),nbVariables,outputVariablesNames,bufferMode);
		break;
	}

	bool valid = m_transformPipeline->link();

}

void GPUTransformFeedback::start(GPUBuffer* toDraw, GLenum primitiveMode, int sizeOfVertex, GLenum typeOfVertex, int stride)
{
	m_PrimitiveMode = primitiveMode;
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_TransformFeedback);

	glBindVertexArray(m_VertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, toDraw->getBuffer());
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, sizeOfVertex, typeOfVertex, GL_FALSE, stride, (void*)0);

	m_transformPipeline->bind();
	glBeginTransformFeedback(primitiveMode);
	
	
}

void GPUTransformFeedback::pause()
{
	glPauseTransformFeedback();
}

void GPUTransformFeedback::resume()
{
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_TransformFeedback);
	glResumeTransformFeedback();
}

void GPUTransformFeedback::draw(int startingOffset, int count)
{
		
	glDrawArrays(m_PrimitiveMode, startingOffset, count);

}

void GPUTransformFeedback::end()
{
	glEndTransformFeedback();
	m_transformPipeline->release();


	glDisableVertexAttribArray(0);
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	
}

GLuint GPUTransformFeedback::getTransformFeedback()
{
	return m_TransformFeedback;
}

void GPUTransformFeedback::disableRasterizer()
{
	glEnable(GL_RASTERIZER_DISCARD);
}

void GPUTransformFeedback::enableRasterizer()
{
	glDisable(GL_RASTERIZER_DISCARD);
}


void GPUTransformFeedback::setTransformFeedbackBuffer(GPUBuffer* recordBuffer, int bindingPoint /*= 0*/)
{
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, m_TransformFeedback);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, bindingPoint, recordBuffer->getBuffer());
}