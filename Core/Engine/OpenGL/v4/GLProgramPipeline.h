/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */

#ifndef _GLPROGRAMPIPELINE_H
#define _GLPROGRAMPIPELINE_H
#include <GL/glew.h>
#include <iostream>
#include <string>
#include <cstdlib>
#include <map>

#include "Engine/OpenGL/v4/GLProgram.h"
#include "./../../Base/Resource_mgr.hpp"

using namespace std;

class  GLProgramPipeline
{
	public:

		GLProgramPipeline(string name = "");

		~GLProgramPipeline();

		void addProgram(GLProgram *p);
		void useProgramStage(GLenum programType, GLProgram *p);
		bool link();
		void bind();
		void release();
		bool validate();

		string info_text;

		void printInfoLog();
		GLuint getProgram();

		GLProgram* vertex;
		GLProgram* tessControl;
		GLProgram* tessEvaluation;
		GLProgram* geometry;
		GLProgram* fragment;
		GLProgram* compute;
	private:
		GLuint m_Pipeline;
		std::string m_Name;

		






};





#endif
