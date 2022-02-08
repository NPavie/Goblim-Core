#include "BilateralFilter.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Engine.h"


BilateralFilter::BilateralFilter(std::string name,int bSize, int size):
	EffectGL(name,"BilateralFilter")
{
	fboSize = size;
	blurSize = bSize;
	blurVertical = new GLProgram(this->m_ClassName+"-Vertical",GL_FRAGMENT_SHADER);
	blurHorizontal = new GLProgram(this->m_ClassName+"-Horizontal",GL_FRAGMENT_SHADER);
	vp = new GLProgram(this->m_ClassName+"-Base",GL_VERTEX_SHADER);


	m_ProgramPipeline->useProgramStage(GL_VERTEX_SHADER_BIT,vp);

	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT,blurVertical);
	m_ProgramPipeline->link();

	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT,blurHorizontal);
	m_ProgramPipeline->link();


	texSamplerH = blurHorizontal->uniforms()->getGPUsampler("fboIn");	
	texSamplerH->Set(0);
	texSamplerV = blurVertical->uniforms()->getGPUsampler("fboIn");	
	texSamplerV->Set(0);

	texSizeH = blurHorizontal->uniforms()->getGPUfloat("texSize");
	texSizeV = blurVertical->uniforms()->getGPUfloat("texSize");

	temp = new GPUFBO("FBO-BilateralFilter");
	temp->create(fboSize,fboSize,1,false);

	

}
BilateralFilter::~BilateralFilter()
{
	temp->destroy();
	delete temp;

	delete blurVertical;
	delete blurHorizontal;
	delete vp;
}


void BilateralFilter::apply(GPUFBO *in)
{

	glDisable(GL_DEPTH_TEST);
	if (m_ProgramPipeline)
	{
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT,blurHorizontal);
		temp->enable();
		texSizeH->Set(in->getWidth());
			in->bindColorTexture(0);
			m_ProgramPipeline->bind();	
			quad->drawGeometry(GL_TRIANGLES);
			m_ProgramPipeline->release();
			in->releaseColorTexture();
		temp->disable();

		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT,blurVertical);
		
		in->enable();
			texSizeV->Set(temp->getHeight());
			temp->bindColorTexture(0);
			m_ProgramPipeline->bind();
			quad->drawGeometry(GL_TRIANGLES);
			m_ProgramPipeline->release();
			temp->releaseColorTexture();
		in->disable();
		
		
	}

	glEnable(GL_DEPTH_TEST);
	


}
