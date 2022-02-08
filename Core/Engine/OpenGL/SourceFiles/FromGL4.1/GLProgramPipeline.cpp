/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */

#include "Engine/OpenGL/GLProgramPipeline.h"
#include "Engine/OpenGL/Managers/textfile.h"

GLProgramPipeline::GLProgramPipeline(std::string name):
	info_text(""),m_Name(name),vertex(NULL),tessControl(NULL),tessEvaluation(NULL),geometry(NULL),fragment(NULL),compute(NULL)
{
	glGenProgramPipelines(1, &m_Pipeline);
	glBindProgramPipeline(m_Pipeline);
}
GLProgramPipeline::~GLProgramPipeline()
{
	glDeleteProgramPipelines(1, &m_Pipeline);
}
void GLProgramPipeline::useProgramStage(GLenum programType, GLProgram *p)
{
	if (p != NULL)
		glUseProgramStages(m_Pipeline, programType, p->getProgram());

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
	else if (programType == GL_COMPUTE_SHADER_BIT)
		compute = p;
	
}

bool GLProgramPipeline::link()
{
    
	GLint param = GL_TRUE;
    /*
	glValidateProgramPipeline(m_Pipeline);
	glGetProgramPipelineiv(m_Pipeline, GL_VALIDATE_STATUS, &param);
	if(param == GL_FALSE)
	{
		printInfoLog();
		throw std::logic_error(string("Pipeline ") + m_Name + string(" : \n") + info_text + "\n");
	}
     */
	return param == GL_TRUE;
}
void GLProgramPipeline::bind()
{

	glBindProgramPipeline(m_Pipeline);

	// Also Bind All Corresponding buffers and samplers

	if (vertex != NULL)
	{
		vertex->uniforms()->bindUniformBuffers();
		
		for ( map<string , GPUsampler* >::iterator it = vertex->uniforms()->listSamplers.begin();it != vertex->uniforms()->listSamplers.end();++it)
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
	if (compute != NULL)
		{
			
		compute->uniforms()->bindUniformBuffers();
		
		for ( map<string , GPUsampler* >::iterator it = compute->uniforms()->listSamplers.begin();it != compute->uniforms()->listSamplers.end();++it)
			it->second->Set();
			
				
	}

}
void GLProgramPipeline::release()
{
	glBindProgramPipeline(0);
}
void GLProgramPipeline::printInfoLog()
{

	int infologLength = 0;
	char *infoLog;

	glGetProgramPipelineiv (m_Pipeline, GL_INFO_LOG_LENGTH,&infologLength);
	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetProgramPipelineInfoLog(m_Pipeline, infologLength, NULL, infoLog);
		string df(infoLog);
		info_text += df ;
		info_text += "\n";
		free(infoLog);
	}
}


GLuint GLProgramPipeline::getProgram()
{
	return m_Pipeline;
}
