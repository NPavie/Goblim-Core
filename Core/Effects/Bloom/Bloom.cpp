#include "Bloom.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Engine.h"



Bloom::Bloom(std::string name,int finalsize,float vbloomTreshold):
	EffectGL(name,"Bloom")
{
	numBuffers = 4;
	fboSize = finalsize;
	vp = new GLProgram(this->m_ClassName+"-Base",GL_VERTEX_SHADER);

	bloom = new GLProgram(this->m_ClassName+"-Bloom",GL_FRAGMENT_SHADER);
	copy = new GLProgram(this->m_ClassName+"-Copy",GL_FRAGMENT_SHADER);
	pass = new GLProgram(this->m_ClassName+"-Pass",GL_FRAGMENT_SHADER);

	//filter = new GLProgram(this->m_ClassName+"-Filter",GL_FRAGMENT_SHADER);

	m_ProgramPipeline->useProgramStage(GL_VERTEX_SHADER_BIT,vp);
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT,bloom);
	m_ProgramPipeline->link();

	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT,copy);
	m_ProgramPipeline->link();

	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT,pass);
	m_ProgramPipeline->link();

	bloomTreshold = copy->uniforms()->getGPUfloat("bloomTreshold");
	bloomTreshold->Set(vbloomTreshold);
	m_bloomTreshold = vbloomTreshold;
	fboIn = pass->uniforms()->getGPUsampler("fboIn");	
	fboIn->Set(0);

	bloomSampler = bloom->uniforms()->getGPUsampler("fboIn");	
	bloomSampler->Set(0);

	bloomSampler1 = bloom->uniforms()->getGPUsampler("fboIn1");	
	bloomSampler1->Set(1);

	bloomSampler2 = bloom->uniforms()->getGPUsampler("fboIn2");	
	bloomSampler2->Set(2);

	bloomSampler3 = bloom->uniforms()->getGPUsampler("fboIn3");	
	bloomSampler3->Set(3);

	int bsize = fboSize;

	for(int i = 0;i < numBuffers;i++)
	{
		bBuffers[i] = new GPUFBO("temp-bloomBuffer");
		bBuffers[i]->create(bsize,bsize,1,false);	

		//Create Blur effect using bilateral filters with adequate size for performances
		blur[i] = new BilateralFilter("Bloom-BlurFilter",9,bsize);
		bsize/= 2;
	}
}
Bloom::~Bloom()
{
	delete temp;
	for (int i = 0;i < numBuffers;i++)
	{
		delete bBuffers[i];
		delete blur[i];
	}
	delete copy;
	delete bloom;
	delete vp;
	delete pass;
}


void Bloom::apply(GPUFBO *in,GPUFBO *out)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_DEPTH_TEST);
	if (m_ProgramPipeline)
	{
			m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT,copy);
			bloomTreshold->Set(m_bloomTreshold);
			bBuffers[0]->enable();
			in->bindColorTexture(0);

			m_ProgramPipeline->bind();	
			quad->drawGeometry(GL_TRIANGLES);
			m_ProgramPipeline->release();
			in->releaseColorTexture();
			
			bBuffers[0]->disable();

			// copy and downsize buffers :  buffers are filled by downsampling precedent buffer, the blurred using adequately sized blur filter
			m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT,pass);
			for (int i = 1;i < numBuffers;i++)
			{

				bBuffers[i]->enable();
				bBuffers[i-1]->bindColorTexture(0);
				m_ProgramPipeline->bind();
				quad->drawGeometry(GL_TRIANGLES);
				m_ProgramPipeline->release();
				bBuffers[i]->disable();
				bBuffers[i-1]->releaseColorTexture();
				blur[i]->apply(bBuffers[i]);

			}

								

		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT,bloom);
		out->enable();

		bBuffers[0]->bindColorTexture(0);
		bBuffers[1]->bindColorTexture(1);
		bBuffers[2]->bindColorTexture(2);
		bBuffers[3]->bindColorTexture(3);

			m_ProgramPipeline->bind();
			quad->drawGeometry(GL_TRIANGLES);
			m_ProgramPipeline->release();
		
		bBuffers[0]->releaseColorTexture();
		bBuffers[1]->releaseColorTexture();
		bBuffers[2]->releaseColorTexture();
		bBuffers[3]->releaseColorTexture();
		out->disable();
		
		
	}
	glEnable(GL_DEPTH_TEST);
	glPopAttrib();
	


}
void Bloom::setBloomTreshold(float v)
{
	bloomTreshold->Set(v);
}