/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */

#ifndef _GLPROGRAMSOURCE_H
#define _GLPROGRAMSOURCE_H
#include <glad/glad.h>
#include <string>
#include <stdexcept>
using namespace std;
#include "Engine/OpenGL/Managers/GPUVariable.hpp"
#include "Engine/OpenGL/Managers/GLUniformManager.h"
class GLProgramSource
{
	public:

		GLProgramSource(string name);

		~GLProgramSource();

		void setFilename(string pathfilename);
		bool createProgram(GLenum shaderType, std::string filename);
        /*
         @brief check if the current program is valid. 
         If the function is used to check the compilation, inCurrentOpenGLState must be set to false
         @param inCurrentOpenGLState check if the program can be executed with the current openGL state. This must be set to false if only the compilation is tested.
         @return True if the program is correctly linked or is valid for execution in the current state of opengl context
         */
		bool isValid(bool inCurrentOpenGLState = false);
		GLuint getProgram();
		string info_text;


		bool loaded;

		// Gestion des named string en static
		static void createNamedString(string name,string filename);
		static void deleteNamedString(string name);
		static bool noIncludes;
		static map<string, const char*> m_IncludeSource;

	private:

		GLenum shaderType;

		string shaderSource;
		vector<string> listOfIncludeFiles;
		vector<const char*> listOfSources;
		vector<int>	listOfLength;

		string m_Filename;
		GLuint m_Program;
		string printErrorString();
    
        void getProgramInfoLog();
		void getShaderInfoLog(GLuint shader);
		void parseShader(std::string parsedSource);
};





#endif
