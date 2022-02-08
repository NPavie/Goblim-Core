/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */

#ifndef _GLPROGRAMSOURCE_H
#define _GLPROGRAMSOURCE_H
#include <GL/glew.h>
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
		bool isValid();
		GLuint getProgram();
		string info_text;
		void printProgramInfoLog();

		bool loaded;

		// Gestion des named string en static
		static void createNamedString(string name,string filename);
		static void deleteNamedString(string name);
		static bool noIncludes;
		static map<string, const char*> m_IncludeSource;
		
		void parseSources(string analysedSource, vector<const char*>* ptrlistOfSources, vector<int>* ptrlistOfLength);

	private:

		GLenum shaderType;

		string shaderSource;
		vector<string> listOfIncludeFiles;
		vector<const char*> listOfSources;
		vector<int>	listOfLength;

		string m_Filename;
		GLuint m_Program;
		string printErrorString();
		void printShaderInfoLog(GLuint shader);
};





#endif
