#include "LightScattering.h"
#include "Engine/Base/Scene.h"


LightScattering::LightScattering(std::string name, int size,float exposure,float decay,float density,float weight):
	EffectGL(name,"LightScattering")
{
	
	vp = new GLProgram(this->m_ClassName+"-Base",GL_VERTEX_SHADER);
	pass = new GLProgram(this->m_ClassName+"-Pass",GL_FRAGMENT_SHADER);
	lightScatter = new GLProgram(this->m_ClassName+"-LightScattering",GL_FRAGMENT_SHADER);

	m_ProgramPipeline->useProgramStage(GL_VERTEX_SHADER_BIT,vp);

	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT,pass);
	m_ProgramPipeline->link();
	
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT,lightScatter);
	m_ProgramPipeline->link();



	fboIn = pass->uniforms()->getGPUsampler("fboIn");	
	fboIn->Set(0);


	lightBuffer = lightScatter->uniforms()->getGPUsampler("lightBuffer");	
	lightBuffer->Set(0);

	this->exposure = lightScatter->uniforms()->getGPUfloat("exposure");
	this->decay = lightScatter->uniforms()->getGPUfloat("decay");
	this->weight = lightScatter->uniforms()->getGPUfloat("weight");
	this->density = lightScatter->uniforms()->getGPUfloat("density");
	this->setExposure(exposure);
	this->setDecay(decay);
	this->setWeight(weight);
	this->setDensity(density);

	this->lightScreenPos = lightScatter->uniforms()->getGPUvec2("lightPositionOnScreen");

	lightScatterBuffer = new GPUFBO("LightScattering-FBO");
	lightScatterBuffer->create(size,size,1,false);

	
}
LightScattering::~LightScattering()
{
	delete lightScatterBuffer;
	delete vp;
	delete pass;
	delete lightScatter;
}

void LightScattering::apply(GPUFBO *fbo,glm::vec2 lightPos)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_DEPTH_TEST);
	if (m_ProgramPipeline)
	{
		// Copy in buffer to light scattering buffer
		lightScatterBuffer->enable();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT,pass);

		fbo->bindColorTexture(0);

		m_ProgramPipeline->bind();			
		quad->drawGeometry(GL_TRIANGLES);
		m_ProgramPipeline->release();
		lightScatterBuffer->disable();
		fbo->releaseColorTexture();

		// Draw result into fbo using light scattering shader
		fbo->enable();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT,lightScatter);

		lightScatterBuffer->bindColorTexture(0);
		lightScreenPos->Set(lightPos*0.5f+0.5f);

		m_ProgramPipeline->bind();			
		quad->drawGeometry(GL_TRIANGLES);
		m_ProgramPipeline->release();
		lightScatterBuffer->releaseColorTexture();
		fbo->disable();


	}
	glPopAttrib();
}


void LightScattering::setDensity(float v)
{
	density->Set(v);
}
void LightScattering::setExposure(float v)
{
	exposure->Set(v);
}
void LightScattering::setDecay(float v)
{
	decay->Set(v);
}
void LightScattering::setWeight(float v)
{
	weight->Set(v);
}
