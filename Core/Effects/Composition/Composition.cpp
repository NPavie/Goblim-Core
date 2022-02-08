#include "Composition.h"
#include "Engine/Base/Scene.h"


Composition::Composition(std::string name, int size):
	EffectGL(name,"Composition")
{
	
	vp = new GLProgram(this->m_ClassName+"-Base",GL_VERTEX_SHADER);
	fp = new GLProgram(this->m_ClassName+"-ColorBloom",GL_FRAGMENT_SHADER);

	m_ProgramPipeline->useProgramStage(GL_VERTEX_SHADER_BIT,vp);
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT,fp);
	m_ProgramPipeline->link();
	m_ProgramPipeline->bind();

	//colorSampler = fp->uniforms()->getGPUsampler("inColor");	
	//colorSampler->Set(0);
	/*
	bloomSampler = fp->uniforms()->getGPUsampler("inBloom");	
	bloomSampler->Set(1);
	
	LightScatterSampler = fp->uniforms()->getGPUsampler("inLightScatter");	
	LightScatterSampler->Set(2);
	
	ssaoSampler = fp->uniforms()->getGPUsampler("inSSAO");	
	ssaoSampler->Set(3);
	*/
	//depthSampler = fp->uniforms()->getGPUsampler("inDepth");	
	//depthSampler->Set(1);

	//bloomAmount = fp->uniforms()->getGPUfloat("bloomAmount");
	//bloomAmount->Set(0.2);
	
	
	
	colorFbo = Scene::getInstance()->getResource<GPUFBO>("colorFBO");
	//bloomFbo = Scene::getInstance()->getResource<GPUFBO>("bloomFBO");
	//lightFbo = Scene::getInstance()->getResource<GPUFBO>("lightFBO");
	//ssaoFbo = Scene::getInstance()->getResource<GPUFBO>("ssaoFBO");

}
Composition::~Composition()
{
	Scene::getInstance()->releaseResource(bloomFbo);
	Scene::getInstance()->releaseResource(colorFbo);
	Scene::getInstance()->releaseResource(lightFbo);
	Scene::getInstance()->releaseResource(ssaoFbo);
	delete fp;
	delete vp;
}

void Composition::apply()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_DEPTH_TEST);
	if (m_ProgramPipeline)
	{
		colorFbo->bindColorTexture(0);
		colorFbo->bindDepthTexture(1);
	
		//bloomFbo->bindColorTexture(1);

		//lightFbo->bindColorTexture(2);

		//glActiveTexture(GL_TEXTURE3);
		//glBindTexture(GL_TEXTURE_2D,ssaoFbo->getColorBuffer());
		
		

		m_ProgramPipeline->bind();			
		quad->drawGeometry(GL_TRIANGLES);
 		m_ProgramPipeline->release();

		colorFbo->releaseColorTexture();
		colorFbo->releaseDepthTexture();
		//bloomFbo->releaseColorTexture();
		//lightFbo->releaseColorTexture();


	}
	glPopAttrib();
}

