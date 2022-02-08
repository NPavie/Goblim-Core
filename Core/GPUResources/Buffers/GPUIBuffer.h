/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */

#ifndef _GPUIBuffer_H
#define _GPUIBuffer_H
#include <GL/glew.h>
#include <string>
#include "Engine/Base/Resource.h"
using namespace std;


class GPUIBuffer : public Resource
{
	public:

		GPUIBuffer(string name);
		~GPUIBuffer();

		void create(GLenum target, GLsizeiptr size,const GLvoid * data, GLbitfield flags);
		GLuint m_BlockIndex;

		GLuint getBuffer();
		void destroy();
		virtual void bind();
		virtual void bind(int bindingPoint);
		virtual void release();

		
	private :
		GLenum m_Target;
		GLuint m_Buffer;
		GLint m_Size;
		GLuint m_BindingPoint;


};





#endif
