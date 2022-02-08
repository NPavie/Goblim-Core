#ifndef _GPUTransformFeedback_h
#define _GPUTransformFeedback_h

#include <GL/glew.h>
#include "Engine/Base/Resource.h"
#include "Engine/OpenGL/v4/GLProgramPipeline.h"
#include "GPUResources/Buffers/GPUBuffer.h"

class GPUTransformFeedback : public Resource
{
public:
	GPUTransformFeedback(std::string name);
	~GPUTransformFeedback();


	/**
	*	@brief	Create a transform feedback object
	**/
	bool create();

	/**
	*	@brief Assign a buffer to the transform feedback object as Transform Feedback buffer
	**/
	void setTransformFeedbackBuffer(GPUBuffer* recordBuffer, int bindingPoint = 0);

	/*
	*	@brief destroy the transform feedback object
	**/
	void destroy();

	/**
	*	@brief Set the pipeline to retrieve transform feedback information
	*
	*	@param	outputVariablesNames		list of names of the variables used to store the pipeline outputs
	*	@param	nbVariables					number of variable in the output variables list
	*	@param 	varyingsStage				index of the stage where the varyings variable are to be captured (0 = Vertex shader .. 3 = Tesselation Evaluation shader)
	*	@param	bufferMode					feedback buffer mode : GL_SEPARATE_ATTRIBS or GL_INTERLEAVED_ATTRIBS (multiple output stream for one feedback rendering pipeline) 
	*	@param	vertexShader				Vertex program to use in the transform feedback pipeline
	*	@param	geometryShader				Geometry program to use in the transform feedback pipeline
	*	@param	tesselationControlShader	TessConstrol shader to use in the transform feedback pipeline
	*	@param	tesselationEvalShader		TessConstrol shader to use in the transform feedback pipeline
	**/
	void setPipeline(
		const char* outputVariablesNames[],
		int nbVariables,
		int varyingsStage,
		GLenum	bufferMode,
		GLProgram* vertexShader,
		GLProgram* geometryShader = NULL,
		GLProgram* tesselationControlShader = NULL,
		GLProgram* tesselationEvalShader = NULL
		);
	
	/**
	*	@brief Start the transform feedback mode
	*
	*	@param	toDraw	buffer of data to use (will be bound as GL_ARRAY_BUFFER)
	*	@param	primitiveMode Type of primitive to be captured : GL_POINTS or GL_LINES or GL_TRIANGLES
	*	@param	sizeOfVertex	Number of components used to describe a vertex (2, 3 or 4)
	*	@param	typeOfVertex	Type of components used to describe a vertex (GL_FLOAT or ...)
	*	@param	stride			memory space between 2 consecutive vertex
	*
	**/
	void start(GPUBuffer* toDraw, GLenum primitiveMode, int sizeOfVertex, GLenum typeOfVertex, int stride);


	/**
	*	@brief Draw a buffer of vertex(bound as an Array buffer) for capture
	*	@param	startingOffset		starting position of the element to draw in the array
	*	@param	count				Number of element to render inside the array buffer
	*
	**/
	void draw(int startingOffset, int count);

	void pause();

	void resume();

	void end();

	void toString();

	GLuint getTransformFeedback();

	void disableRasterizer();

	void enableRasterizer();

private:
	
	GLuint				m_VertexArray;

	// Transform feedback object ID
	GLuint				m_TransformFeedback;

	// Transform feedback primitive mode
	GLenum				m_PrimitiveMode;				

	// Pipeline to apply transform feedback
	GLProgramPipeline*	m_transformPipeline;

	// Buffer to use as input
	GPUBuffer*			m_DrawBuffer;

	// Buffer to store the output of transform feedback
	GPUBuffer*			m_RecordBuffer;
	
	// Varyings list
	vector<std::string>	m_Varyings;


};

#endif
