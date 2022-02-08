/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */

#ifndef _GLPROGRAMSSOURCEMANAGER_H
#define _GLPROGRAMSSOURCEMANAGER_H
#include <GL/glew.h>
#include <string>
#include <vector>
using namespace std;
#include "Engine/OpenGL/v4/GLProgramSource.h"
#include "Engine/Base/Resource_mgr.hpp"






// Resource manager for various programs source
// Responsible for browsing directories and instanciating all GLProgramSource
// Using AddPath from Material
// should concatenate class_name with shader_name
class GLProgramsSourceManager
{
	public:
		GLProgramsSourceManager();
		~GLProgramsSourceManager();

		void addPath(string name,string className ="");

		GLProgramSource* get(string name,GLenum type = 0);
		void release(string name,GLenum type = 0);

		string info_text;
		

	protected:
		vector<string> m_Paths;

		// Factory for program instanciation
		Resource_mgr<GLProgramSource> m_Vertex;
		Resource_mgr<GLProgramSource> m_TessControl;
		Resource_mgr<GLProgramSource> m_TessEvaluation;
		Resource_mgr<GLProgramSource> m_Geometry;
		Resource_mgr<GLProgramSource> m_Fragment;
		Resource_mgr<GLProgramSource> m_Compute;

		// Map for pairing program name with available glsl filename. Indications of available source code for shader generation
		map<string,string> m_listVertex;
		map<string,string> m_listTessControl;
		map<string,string> m_listTessEvaluation;
		map<string,string> m_listGeometry;
		map<string,string> m_listFragment;
		map<string,string> m_listCompute;
		map<string,string> m_listNamedStrings;
	private:
		void addFile(string f_path,string name,string cName);
		void addDir(string prefix,string className);


};





#endif
