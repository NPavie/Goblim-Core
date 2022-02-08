#include "SSAO.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Engine.h"


SSAO::SSAO(std::string name, int size):
	EffectGL(name,"SSAO")
{
	scene = Scene::getInstance();
	noise = scene->getResource<GPUTexture2D>("./Textures/normalmap.png");
	normalMat = new NormalMaterial(name+"_NormalMaterial");
	fboSize = size;

	normalFBO = scene->getResource<GPUFBO>("NormalFBO");
	if (normalFBO->isInitialized())
		fboSize = normalFBO->getWidth();
	else
		normalFBO->create(fboSize,fboSize,1,true);

	ssaoFBO = scene->getResource<GPUFBO>("SSAOFBO");
	if (ssaoFBO->isInitialized())
		fboSize = ssaoFBO->getWidth();
	else
		ssaoFBO->create(fboSize,fboSize,1,false);


	
	// Shaders
	vp = new GLProgram(this->m_ClassName+"-Base",GL_VERTEX_SHADER);
	ssaoShader = new GLProgram(this->m_ClassName+"-Base",GL_FRAGMENT_SHADER);


	// Link
	m_ProgramPipeline->useProgramStage(GL_VERTEX_SHADER_BIT,vp);
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT,ssaoShader);
	m_ProgramPipeline->link();
	
	normalMapSampler = ssaoShader->uniforms()->getGPUsampler("normalMap");
	randomMapSampler = ssaoShader->uniforms()->getGPUsampler("randomMap");	
	normalMapSampler->Set(0);
	randomMapSampler->Set(1);

	texelSize = ssaoShader->uniforms()->getGPUvec2("texelSize");
	//texelSize->Set(glm::vec2(1.0/fboSize,1.0/fboSize));
	texelSize->Set(glm::vec2(0.001,0.001));

	occluderBias = ssaoShader->uniforms()->getGPUfloat("occluderBias");
	samplingRadius = ssaoShader->uniforms()->getGPUfloat("samplingRadius");
	attenuation = ssaoShader->uniforms()->getGPUvec2("attenuation");


	setOccluderBias(0.05f);
	setSamplingRadius(20.0f);
	setAttenuation(glm::vec2(1.0,5.0));


	blur = new BilateralFilter("SSAO-Blur");

}

void SSAO::setOccluderBias(float f)
{
	occluderBias->Set(f);
}
void SSAO::setSamplingRadius(float f)
{
	samplingRadius->Set(f);
}
void SSAO::setAttenuation(glm::vec2 f)
{
	attenuation->Set(f);
}

SSAO::~SSAO()
{
	delete ssaoShader;
	delete vp;
	delete normalMat;
	scene->releaseResource("NormalFBO");
	scene->releaseResource("SSAOFBO");
}


void SSAO::apply()
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_DEPTH_TEST);
	
	if (m_ProgramPipeline)
	{		
		
		ssaoFBO->enable();
						// Bind de la carte des normales et de la texture de bruit
		normalFBO->getColorTexture(0)->bind(0);

			glActiveTexture(GL_TEXTURE1);
			glTexParameteri(GL_TEXTURE_2D , GL_TEXTURE_WRAP_S, GL_REPEAT);  
			glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
			noise->bind(1);			
			
			m_ProgramPipeline->bind();
			quad->drawGeometry(GL_TRIANGLES);
			m_ProgramPipeline->release();

			noise->release();
			normalFBO->getColorTexture(0)->release();

		ssaoFBO->disable();
	}

	glPopAttrib();
	


}
void SSAO::computeSSAO(NodeCollector* collector)
{
	
	normalFBO->enable();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	for (unsigned int i = 0;i < collector->nodes.size();i++)
			collector->nodes[i]->render(normalMat);	
	normalFBO->disable();
	apply();
	blur->apply(ssaoFBO);
	
}