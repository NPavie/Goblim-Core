/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */


#include "Engine/OpenGL/GLProgram.h"
#include "Engine/OpenGL/Managers/textfile.h"
#include <iostream>

GLProgramsSourceManager GLProgram::prgMgr = GLProgramsSourceManager();


GLProgram::GLProgram(std::string name,GLenum type):
m_Name(name),m_Type(type)
{
	info_text = "";
	src = prgMgr.get(name,type);
	m_UniformMgr = NULL;
	linkedProgramAvailable = false;

	
}
GLProgram::~GLProgram()
{
	if (src != NULL)
		prgMgr.release(m_Name,m_Type);
	if (m_UniformMgr != NULL)
		delete m_UniformMgr;

}
GLUniformManager* GLProgram::uniforms()
{
	return m_UniformMgr;
}

bool GLProgram::isValid()
{
		return(src != NULL && src->isValid());
}
GLuint GLProgram::getProgram()
{
	if(linkedProgramAvailable)	return(linkedProgram);
	else throw std::logic_error("In GLProgram " + this->getName() + " : no linked program found \n");
}
GLuint GLProgram::getShader()
{
	if (src != NULL)
		return(src->getProgram());
	else
		return -1;
}
GLenum GLProgram::getType()
{
	return m_Type;
}
void GLProgram::updateManager(GLuint program)
{
	linkedProgram = program;
	linkedProgramAvailable = true;
	if (src)
		m_UniformMgr = new GLUniformManager(this);


}

string GLProgram::getName()
{
	string name = m_Name;
	switch (m_Type)
	{
	case GL_VERTEX_SHADER:
		name += "-VS";
		break;
	case GL_FRAGMENT_SHADER:
		name += "-FS";
		break;
	case GL_GEOMETRY_SHADER:
		name += "-GS";
		break;
	case GL_TESS_CONTROL_SHADER:
		name += "-TS";
		break;
	case GL_TESS_EVALUATION_SHADER:
		name += "-ES";
		break;
	case GL_COMPUTE_SHADER:
		name += "-CS";
		break;
	default:
		name += "Unknown";
	}

	return name;
}
