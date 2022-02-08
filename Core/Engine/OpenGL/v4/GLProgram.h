/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */

#ifndef _GLPROGRAM_H
#define _GLPROGRAM_H
#include <GL/glew.h>
#include <string>
#include <stdexcept>
using namespace std;
#include "Engine/OpenGL/Managers/GPUVariable.hpp"
#include "Engine/OpenGL/Managers/GLUniformManager.h"
#include "Engine/OpenGL/v4/GLProgramSource.h"
#include "Engine/OpenGL/Managers/GLProgramsSourceManager.h"
class GLProgram
{
	public:

		GLProgram(std::string name,GLenum type);
		~GLProgram();

		bool isValid();
		GLuint getProgram();
		GLenum getType();
		string info_text;
		GLUniformManager* uniforms();
		static GLProgramsSourceManager prgMgr;
		string getName();
	private:
		string m_Name;
		GLenum m_Type;
		GLProgramSource* src;
		GLUniformManager *m_UniformMgr;


};





#endif
