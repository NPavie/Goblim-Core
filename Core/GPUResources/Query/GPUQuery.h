#ifndef GPU_QUERY_H
#define GPU_QUERY_H

#include <GL/glew.h>
#include <string>
#include "Engine/Base/Resource.h"
using namespace std;


class GPUQuery : public Resource
{
public :

	GPUQuery(string name, GLenum queryType);
	~GPUQuery();

	/**
	*	@brief create a query object
	*/
	void create();

	/**
	*	@brief start the query process
	*
	*	@param queryType	type of value to query : 
	*		- GL_SAMPLES_PASSED :  The number of samples that pass the depth test for all rendering commands within the scope of the query
	*		- GL_ANY_SAMPLES_PASSED : GL_FALSE if none of the scoped rendering commands generate samples that pass the depth test, otherwise, the value is GL_TRUE
	*		- GL_ANY_SAMPLES_PASSED_CONSERVATIVE : Same as GL_SAMPLES_PASSED, except that the implementation might use a less accurate but faster algorithm
	*		- GL_PRIMITIVES_GENERATED : Records the number of primitives sent to a Geometry Shader (or 0 if no GS active) 
	*		- GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN : Records the number of primitives written by a Geometry Shader to a Transform Feedback Object 
	*		- GL_TIME_ELAPSED : Records the time that it takes for the GPU to execute the scoped commands
	*		- GL_TIMESTAMP : Records the current time of GPU
	*/
	void begin();
	
	/**
	*	@brief stop the query process
	*/
	void end();

	GLint getResultInt();
	GLint64 getResultInt64();
	GLuint getResultUInt();
	GLuint64 getResultUInt64();
	

	void destroy();
private :

	GLenum m_queryType;
	GLuint m_queryId[2][1];	// from lighthouse 3D : double query buffer to avoid application stall
	GLuint m_queryBufferFront;
	GLuint m_queryBufferBack;

	void*  m_queryResult;
	bool m_updateResult;

};


#endif