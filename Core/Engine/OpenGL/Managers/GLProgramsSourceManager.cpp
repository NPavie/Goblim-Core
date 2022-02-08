#include "Engine/OpenGL/Managers/GLProgramsSourceManager.h"

#include <iostream>
#ifdef WIN32
#include "Utils/dirent.h"
#else
#include <dirent.h>
#endif

#include <stdio.h>
#include <string.h>

string MainMaterialPath = "./Materials";
/*
bool isDir(char* s)
{
	return (strchr(s, '.') == NULL);
}
*/

GLProgramsSourceManager::GLProgramsSourceManager()
{

}
GLProgramsSourceManager::~GLProgramsSourceManager()
{
	for( map<string , string >::iterator it = m_listNamedStrings.begin();it != m_listNamedStrings.end();it++)
		GLProgramSource::deleteNamedString(it->first);
}

void GLProgramsSourceManager::addPath(string path,string className)
{
	DIR *dir;
	struct dirent *ent;
	dir = opendir(path.c_str());
	if (dir != NULL) {
		while ((ent = readdir(dir)) != NULL)
		{
			if (strchr(ent->d_name, '.') == NULL)
				addDir(path +"/"+ent->d_name, className);
			else
				addFile(path+"/", ent->d_name,className);



		}
		closedir (dir);

	}
}
void GLProgramsSourceManager::addDir(string prefix,string className)
{
	DIR *dir;
	struct dirent *ent;
	dir = opendir ((prefix ).c_str());
	if (dir != NULL) {
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL)
		{
			if (strchr(ent->d_name, '.') == NULL)
				addDir(prefix+"/"+ent->d_name,className);
			else
				addFile(prefix+"/",ent->d_name,className);
		}
		closedir (dir);

	}
}
void GLProgramsSourceManager::addFile(string f_path,string name,string cName)
{

	int ext = name.rfind (".glsl");
	if (ext != string::npos)
	{
		string Nname = name;
		Nname.erase(ext);
		int typ = Nname.find_last_of("-");
		/*
		if (typ ==  string::npos)
		{

			string finalName = f_path + Nname;
			while (finalName.find_first_of(".") == 0) // Removing . and starting at first /
				finalName = finalName.substr(1,string::npos) ;
			if (m_listNamedStrings.find(finalName) == m_listNamedStrings.end()) // Search Existing list
			{
				m_listNamedStrings.insert(make_pair(finalName ,f_path+name));				// Not present ? : insert into list and load to GPU memory
				GLProgramSource::createNamedString(finalName, f_path+name);
			}

		}*/
		if (typ == string::npos)
		{

			string finalName = f_path + Nname;
			//while (finalName.find_first_of(".") == 0) // Removing . and starting at first /
				//finalName = finalName.substr(1,string::npos) ;

			// Removing rootPath from material path : 
			unsigned pos = finalName.find("/Material");
			finalName = finalName.substr(pos) ;

			if (m_listNamedStrings.find(finalName) == m_listNamedStrings.end()) // Search Existing list
			{
				LOG(TRACE) << "Loading : " << finalName << endl;
				
				m_listNamedStrings.insert(make_pair(finalName ,f_path+name));				// Not present ? : insert into list and load to GPU memory
				GLProgramSource::createNamedString(finalName, f_path+name);
			}

		}
		else
		{
			string type = name.substr(typ+1,2);			// Removing -VS,-FS...
			string mainName = name.erase(typ);
			if (!type.compare("VS"))
				m_listVertex.insert(make_pair(cName+"-"+mainName ,f_path+name+"-VS.glsl" ));
			else if (!type.compare("TS"))
				m_listTessControl.insert(make_pair(cName+"-"+mainName ,f_path+name+"-TS.glsl" ));
			else if (!type.compare("ES"))
				m_listTessEvaluation.insert(make_pair(cName+"-"+mainName ,f_path+name+"-ES.glsl" ));
			else if (!type.compare("GS"))
				m_listGeometry.insert(make_pair(cName+"-"+mainName ,f_path+name+"-GS.glsl" ));
			else if (!type.compare("FS"))
				m_listFragment.insert(make_pair(cName+"-"+mainName ,f_path+name+"-FS.glsl" ));
			else if (!type.compare("CS"))
				m_listCompute.insert(make_pair(cName+"-"+mainName ,f_path+name+"-CS.glsl" ));
			else
				LOG(ERROR) << "unkown shader type for file " <<  f_path+name << endl;
		}

	}



}


GLProgramSource* GLProgramsSourceManager::get(string name,GLenum type)
{
	GLProgramSource* to_ret = NULL;
	Resource_mgr<GLProgramSource> *pgrmgr = NULL;
	map<string,string> *listmgr = NULL;
	if (type==GL_VERTEX_SHADER)
	{
		pgrmgr = &m_Vertex;
		listmgr = &m_listVertex;
	}
	else if (type==GL_TESS_CONTROL_SHADER)
	{
		pgrmgr = &m_TessControl;
		listmgr = &m_listTessControl;
	}
	else if (type==GL_TESS_EVALUATION_SHADER)
	{
		pgrmgr = &m_TessEvaluation;
		listmgr = &m_listTessEvaluation;
	}
	else if (type==GL_GEOMETRY_SHADER)
	{
		pgrmgr = &m_Geometry;
		listmgr = &m_listGeometry;
	}
	else if (type==GL_FRAGMENT_SHADER)
	{
		pgrmgr = &m_Fragment;
		listmgr = &m_listFragment;
	}
	else if (type==GL_COMPUTE_SHADER)
	{
		pgrmgr = &m_Compute;
		listmgr = &m_listCompute;
	}
	else
		throw std::logic_error("ERROR : GLProgramsSourceManager : Error retrieving program : type unknown\n");


	if (listmgr->find(name) != listmgr->end())
	{
		to_ret = pgrmgr->get(name);
		if (!to_ret->loaded)
		{
			try
			{
				to_ret->createProgram(type,listmgr->find(name)->second);
			}
			catch(const std::exception & e )
			{
				pgrmgr->release(name);
				to_ret = NULL;
				throw std::logic_error("ERROR : GLProgramsSourceManager : " + string(e.what()) );
			}
		}
	}
	else
	{
		string typeString;
		switch (type)
		{
		case GL_VERTEX_SHADER:
			typeString = "Vertex ";
			break;
		case GL_GEOMETRY_SHADER:
			typeString = "Geometry ";
			break;
		case GL_TESS_CONTROL_SHADER:
			typeString = "Tesselation Control ";
			break;
		case GL_TESS_EVALUATION_SHADER:
			typeString = "Tesselation Evaluation ";
			break;
		case GL_FRAGMENT_SHADER:
			typeString = "Fragment ";
			break;
		default:
			typeString = "untyped ";
		}
		LOG(TRACE) << " WARNING :: " + typeString + " program " + name + " is unknown : using default stage\n";
		//GLError::sendDebugNotification(string(" WARNING :: " + typeString + " program " + name + " is unknown : using default stage"), true);
		//std::cout << "WARNING : " << typeString << "program " << name << " is unknown : using default stage" << endl;
		//throw std::logic_error("ERROR : GLProgramsSourceManager : Error retrieving program " + name + " : Program Name unknown\n");
	}



	return to_ret;
}

void GLProgramsSourceManager::release(string name,GLenum type)
{

	Resource_mgr<GLProgramSource> *pgrmgr = NULL;
	if (type==GL_VERTEX_SHADER)
		pgrmgr = &m_Vertex;
	else if (type==GL_TESS_CONTROL_SHADER)
		pgrmgr = &m_TessControl;
	else if (type==GL_TESS_EVALUATION_SHADER)
		pgrmgr = &m_TessEvaluation;
	else if (type==GL_GEOMETRY_SHADER)
		pgrmgr = &m_Geometry;
	else if (type==GL_FRAGMENT_SHADER)
		pgrmgr = &m_Fragment;
	else if (type==GL_COMPUTE_SHADER)
		pgrmgr = &m_Compute;
	pgrmgr->release(name);

}
