#include "DepthCopy.h"
#include "Engine/Base/Scene.h"


#include "GPUResources/Textures/GPUTexture2D.h"




DepthCopy::DepthCopy(std::string name) :
EffectGL(name, "DepthCopy")
{

	vp = new GLProgram(this->m_ClassName + "-Base", GL_VERTEX_SHADER);
	fp = new GLProgram(this->m_ClassName + "-DepthCopy", GL_FRAGMENT_SHADER);

	m_ProgramPipeline->useProgramStage(GL_VERTEX_SHADER_BIT, vp);
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, fp);
	m_ProgramPipeline->link();
	m_ProgramPipeline->bind();

	displaybox = vp->uniforms()->getGPUvec4("box");
	res = fp->uniforms()->getGPUsampler("resourceSampler");
	res->Set(0);

	screenBox = glm::vec4(0, 0, 1, 1);


}
DepthCopy::~DepthCopy()
{
	delete fp;
	delete vp;
}
void DepthCopy::display(GPUTexture2D* depthTexture)
{
	depthTexture->bind(res->getValue());
	displaybox->Set(2.0f*screenBox);
	//glPushAttrib(GL_DEPTH_BITS);
	//glDisable(GL_DEPTH_TEST);
	m_ProgramPipeline->bind();
	quad->drawGeometry(GL_TRIANGLES);
	m_ProgramPipeline->release();
	depthTexture->release();
	//glPopAttrib();


}
