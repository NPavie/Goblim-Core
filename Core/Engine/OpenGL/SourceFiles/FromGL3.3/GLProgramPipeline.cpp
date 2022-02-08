/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */

#include "Engine/OpenGL/GLProgramPipeline.h"

GLProgramPipeline::GLProgramPipeline(std::string name):
info_text(""),m_Name(name),vertex(NULL),tessControl(NULL),tessEvaluation(NULL),geometry(NULL),fragment(NULL)
{
	m_Pipeline = glCreateProgram();
}
GLProgramPipeline::~GLProgramPipeline()
{
	glDeleteProgram(m_Pipeline);
}
void GLProgramPipeline::useProgramStage(GLenum programType, GLProgram *p)
{

	glAttachShader(m_Pipeline,p->getShader());


	if (programType == GL_VERTEX_SHADER_BIT)
		vertex = p;
	else if (programType == GL_TESS_CONTROL_SHADER_BIT)
		tessControl = p;
	else if (programType == GL_TESS_EVALUATION_SHADER_BIT)
		tessEvaluation = p;
	else if (programType == GL_GEOMETRY_SHADER_BIT)
		geometry = p;
	else if (programType == GL_FRAGMENT_SHADER_BIT)
		fragment = p;


	printInfoLog();
}

bool GLProgramPipeline::link()
{
	GLint param = GL_FALSE;

	glLinkProgram(m_Pipeline);
	
	glBindAttribLocation(m_Pipeline, 0, "Position");
	glBindAttribLocation(m_Pipeline, 1, "Color");
	glBindAttribLocation(m_Pipeline, 2, "Normal");
	glBindAttribLocation(m_Pipeline, 3, "TexCoord");
	glBindAttribLocation(m_Pipeline, 4, "Tangent");

	glGetProgramiv (m_Pipeline, GL_LINK_STATUS,&param);
	if(param == GL_FALSE)
	{
		printInfoLog();
		throw std::logic_error(string("Pipeline ") + m_Name + string(" : \n") + info_text + "\n");
	}

	if (vertex != NULL)
		vertex->updateManager(m_Pipeline);
	if (tessControl != NULL)
		tessControl->updateManager(m_Pipeline);
	if (tessEvaluation != NULL)
		tessEvaluation->updateManager(m_Pipeline);
	if (geometry != NULL)
		geometry->updateManager(m_Pipeline);
	if (fragment != NULL)
		fragment->updateManager(m_Pipeline);


	return (param == GL_TRUE);

}
void GLProgramPipeline::bind()
{
	glUseProgram(m_Pipeline);

	// Also Bind All Corresponding buffers

	if (vertex != NULL)
	{
		vertex->uniforms()->bindUniformBuffers();
		for (map<string, GPUsampler* >::iterator it = vertex->uniforms()->listSamplers.begin(); it != vertex->uniforms()->listSamplers.end(); ++it)
			it->second->Set();
		
	}
	if (tessControl != NULL)
	{
		tessControl->uniforms()->bindUniformBuffers();
		for ( map<string , GPUsampler* >::iterator it = tessControl->uniforms()->listSamplers.begin();it != tessControl->uniforms()->listSamplers.end();++it)
			it->second->Set();
	}
	if (tessEvaluation != NULL)
	{
		tessEvaluation->uniforms()->bindUniformBuffers();
		for ( map<string , GPUsampler* >::iterator it = tessEvaluation->uniforms()->listSamplers.begin();it != tessEvaluation->uniforms()->listSamplers.end();++it)
			it->second->Set();
	}
	if (geometry != NULL)
	{
		geometry->uniforms()->bindUniformBuffers();
		for ( map<string , GPUsampler* >::iterator it = geometry->uniforms()->listSamplers.begin();it != geometry->uniforms()->listSamplers.end();++it)
			it->second->Set();
	}
	if (fragment != NULL)
	{
		fragment->uniforms()->bindUniformBuffers();
		for ( map<string , GPUsampler* >::iterator it = fragment->uniforms()->listSamplers.begin();it != fragment->uniforms()->listSamplers.end();++it)
			it->second->Set();
	}

}
void GLProgramPipeline::release()
{
	//glUseProgram(0);
}
void GLProgramPipeline::printInfoLog()
{

	int infologLength = 0;
	char *infoLog;
	
	glGetProgramiv(m_Pipeline, GL_INFO_LOG_LENGTH, &infologLength);
	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(m_Pipeline, infologLength, NULL, infoLog);
		string df(infoLog);
		info_text += df;
		info_text += "\n";
		free(infoLog);
	}
}
GLuint GLProgramPipeline::getProgram()
{
	return m_Pipeline;
}
