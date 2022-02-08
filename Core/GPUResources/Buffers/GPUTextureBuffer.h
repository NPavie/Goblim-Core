/*
 * GPUTextureBuffer.h
 *
 *  Created on: 14 déc. 2013
 *      Author: nicolas
 */

#ifndef GPUTEXTUREBUFFER_H_
#define GPUTEXTUREBUFFER_H_


#include "GPUResources/Buffers/GPUBuffer.h"


class GPUTextureBuffer : public GPUBuffer
{
public:
	GPUTextureBuffer();
	GPUTextureBuffer(std::string name);

	~GPUTextureBuffer();
	

	/**
	*	@brief allocate in GPU memory a Texture Buffer (see GL_TEXTURE_BUFFER) 
	*	@warning : Access to the buffer as a texture is disabled by default (textureFormat = GL_NONE)
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
	*	@param textureFormat specify a format for texel access (see table of format at : http://www.opengl.org/sdk/docs/man3/xhtml/glTexBuffer.xml)
	*	
	*/
	bool create(int size, GLenum bufferUsage = GL_STREAM_DRAW, GLint textureFormat = GL_NONE);
	
	void destroy();


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
	*	@brief unMap the buffer being currently mapped
	*/
	void unMap();
	
	/**
	*	@brief bind the current buffer to the GL_TEXTURE_BUFFER buffer state
	*/
	void bind(int bindingPoint = 0);

	/**
	*	@brief release the GL_TEXTURE_BUFFER buffer state
	*/
	void release();


	/**
	*	@brief Transfer the data from the buffer to the texture unit
	*/
	void transferBufferToTexture();

	/**
	*	@brief bind the texture associated to the buffer (if the texture access has been set in constructor)
	*/
	void bindAsTexture(int channel);
	void releaseTexture();

private:
	bool bufferBound;
	GLint m_Format;
	GLuint m_TextureId;

	int bound_on_channel;
};

#endif /* GPUTEXTUREBUFFER_H_ */
