#include "DisplayResource.h"
#include "Engine/Base/Scene.h"


#include "GPUResources/Textures/GPUTexture2D.h"




DisplayResource::DisplayResource(std::string name):
	EffectGL(name,"DisplayResource")
{
	
	vp = new GLProgram(this->m_ClassName+"-Base",GL_VERTEX_SHADER);
	fp = new GLProgram(this->m_ClassName+"-Display",GL_FRAGMENT_SHADER);

	m_ProgramPipeline->useProgramStage(GL_VERTEX_SHADER_BIT,vp);
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT,fp);
	m_ProgramPipeline->link();
	m_ProgramPipeline->bind();

	displaybox = vp->uniforms()->getGPUvec4("box");
	res = fp->uniforms()->getGPUsampler("resourceSampler");
	res->Set(0);

	isUIntDepth = fp->uniforms()->getGPUint("isUIntDepth");
	isUIntDepth->Set(0);


	
}
DisplayResource::~DisplayResource()
{
	delete fp;
	delete vp;
}
void DisplayResource::display(const glm::vec4 & box)
{
		displaybox->Set(2.0f*box);
		glPushAttrib(GL_DEPTH_BITS);
		glDisable(GL_DEPTH_TEST);		
		m_ProgramPipeline->bind();			
		quad->drawGeometry(GL_TRIANGLES);
		m_ProgramPipeline->release();
		glPopAttrib();
		
}
