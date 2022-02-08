/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */

#ifndef _GLPROGRAM_H
#define _GLPROGRAM_H
#include <glad/glad.h>
#include <string>
#include <stdexcept>
using namespace std;
#include "Engine/OpenGL/Managers/GPUVariable.hpp"
#include "Engine/OpenGL/Managers/GLUniformManager.h"
#include "Engine/OpenGL/GLProgramSource.h"
#include "Engine/OpenGL/Managers/GLProgramsSourceManager.h"
class GLProgram
{
	public:

		GLProgram(std::string name,GLenum type);
		~GLProgram();

		bool isValid();

		// Return the opengl program compiled from a shader
		GLuint getProgram();

		GLenum getType();
		string info_text;
		GLUniformManager* uniforms();
		static GLProgramsSourceManager prgMgr;
		string getName();
		
		// For OpenGL 3.3 : initially a program was a simple shader in 3.3, but it became a separable object (a shader binary) in 4.x
		GLuint getShader();

		// For openGL 3.3 : update the uniform manager of a shader program,
		void updateManager(GLuint program);

	private:
		string m_Name;
		GLenum m_Type;
		GLProgramSource* src;
		GLUniformManager *m_UniformMgr;

		// openGL 3.3 variable
		bool linkedProgramAvailable;
		GLuint linkedProgram;


};





#endif
