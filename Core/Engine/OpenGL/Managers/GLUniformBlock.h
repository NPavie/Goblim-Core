/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */

#ifndef _GLUNIFORMBLOCK_H
#define _GLUNIFORMBLOCK_H
#include <GL/glew.h>
#include <string>
#include <stdexcept>
using namespace std;
#include "GPUVariable.hpp"

class GPUBuffer;


////////////////// Block Binding index convention
#define VERTEX_BINDING_START 0
#define GEOMETRY_BINDING_START 5
#define TESS_CONTROL_BINDING_START 10
#define TESS_EVALUATION_BINDING_START 15
#define FRAGMENT_BINDING_START 20
#define LIGHTING_BINDING 25
#define MISC_BINDING_START 26

class GLProgram;

class GLUniformBlock
{
	public:

		GLUniformBlock(string name,GLProgram *program,int index,GLint size,bool shared = false);
		~GLUniformBlock();

		GLuint m_BlockIndex;

		GLuint getUniformBuffer();

		void bindBuffer();
		void bindBuffer(GLint bindingPoint);
		void attachBuffer(GPUBuffer *u);
		GLuint m_BindingPoint;
		bool m_Shared;

		string getName();


	private :
		
		GPUBuffer *ubo;
		string m_Name;
		GLProgram* m_Program;
		int m_Index;
		GLint m_Size;
		


};





#endif
