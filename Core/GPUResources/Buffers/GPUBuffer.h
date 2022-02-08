/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */

#ifndef _GPUBuffer_H
#define _GPUBuffer_H
#include <GL/glew.h>
#include <string>
#include "Engine/Base/Resource.h"
using namespace std;

class GLProgram;

class GPUBuffer : public Resource
{
	public:

		GPUBuffer(string name);
		~GPUBuffer();

		/**
		*	@brief allocate in GPU memory a Buffer of given type
		*	@param	size	size of the buffer
		*	@param	usage	specify the usage of the buffer using a special enum value named GL_AccessFrequency_AccessNature
		*	with AccessFrequency equal to :
		*	- STREAM	: The buffer contents will be modified once and used at most a few times
		*	- STATIC	: The buffer contents will be modified once and used many times
		*	- DYNAMIC	: The buffer contents will be modified repeatedly and modified many times
		*	with AccessNature equal to :
		*	- DRAW		: The buffer contents are modified by the application and used by GL to draw
		*	- READ		: The buffer contents are modified by reading data from GL, and used to return that data when queried
		*	- COPY		: The buffer contents are modified by reading data from GL, used by GL to draw
		*/
		virtual void create(GLint size = 0,GLenum type = GL_UNIFORM_BUFFER,GLenum usage = GL_STREAM_DRAW);

		virtual void setBindingPoint(int bindingPoint);
		/**
		*	@brief map a range of the current buffer to host memory
		*	@param	offset	starting offset within the buffer of the range to be mapped
		*	@param	lengthOfRange	length of the range to be mapped
		*	@param	accessFlags		specify the access policy : 
		*	- GL_MAP_READ_BIT	: the pointer returned can be used to read data from the buffer
		*	- GL_MAP_WRITE_BIT	: the pointer returned can be used to write data in the buffer
		*	- GL_MAP_INVALIDATE_RANGE_BIT (optional)	: previous content of the specified range may be discarded (not to be used with GL_MAP_READ_BIT)
		*	- GL_MAP_INVALIDATE_BUFFER_BIT (optional)	: previous content of the entire uffer may be discarded (not to be used with GL_MAP_READ_BIT)
		*	- GL_MAP_FLUSH_EXPLICIT_BIT (optional)		: one or more discrete subranges of the mapping may be modified. modifications to each subrange must be flushed using glFlushMappedBufferRange (TODO : to overload)
		*	- GL_MAP_UNSYNCHRONIZED_BIT (optional)		: the GL should not attempt to synchronize pending operations on the buffer prior to returning from glMapBufferRange.
		*	
		*	@error GL_INVALID_VALUE	is raised if offset or lengthOfRange is negative, or offset + length > GL_BUFFER_SIZE or accessFlags has any bits set other than those described in param
		*	@error GL_INVALID_OPERATION is raised if :
		*	- Buffer already mapped
		*	- Neither GL_MAP_READ_BIT or GL_MAP_WRITE_BIT is set in accessFlags
		*	- GL_MAP_READ_BIT is set with any of those optional flag set : GL_MAP_INVALIDATE_RANGE_BIT, GL_MAP_INVALIDATE_BUFFER_BIT, GL_MAP_UNSYNCHRONIZED_BIT
		*	- GL_MAP_FLUSH_EXPLICIT_BIT is set in accessFlags without GL_MAP_WRITE_BIT
		*/
		void* mapRange(int offset, int lengthOfRange, GLbitfield accessFlags);

		/**
	*	@brief map the current buffer to host memory
	*	@param accessFlag specify the access policy : must be GL_READ_ONLY or GL_WRITE_ONLY or GL_READ_WRITE
	*	@return a non typed pointer to the first element of the buffer
	*/
	void* map(GLenum accessFlag);

	/**
	*	@brief unMap the buffer being currently mapped
	*/
	void unMap();
	
	/**
	*	@brief (mandatory for AMD) link the buffer binding point to a named GLSL buffer (storage block) in a GLprogram
	*/
	void bindToProgramStorageBlock(GLProgram* program, const char* bufferNameInGPU);

	/**
	*	@brief (mandatory for AMD) link a specified SSBO binding point to a named GLSL buffer (storage block) in a GLprogram
	*/
	static void linkBindingPointToProgramStorageBlock(GLProgram* program, const char* bufferNameInGPU, int bindingPoint);

		GLuint getBuffer();

		GLenum getType();

		void destroy();
		
		/*
         @brief Active the current buffer for use and set its access point in memory
         @param bindingPoint Access point of the buffer
         */
		virtual void bind(int bindingPoint = -1);
		
		virtual void release(int fromBindingPoint = -1);

		virtual void Set(int offsetInByte, int sizeInByte, void* value);

		virtual void Get(int offsetInByte, int sizeInByte, void* value);

		virtual bool isBound();
		
	protected :
		GLenum m_Type;
		GLuint m_Buffer;
		GLint m_Size;
		GLuint m_BindingPoint;
		GLuint m_oldBindingPoint;
		GLuint m_BlockIndex;
		GLint maximumBinding;

		bool m_isBound;
};





#endif
