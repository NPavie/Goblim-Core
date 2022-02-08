#include "Effects/SampleEffect/SampleEffect.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Engine.h"



SampleEffect::SampleEffect(std::string name) :
EffectGL(name, "SampleEffect")
{
	/* Default Vertex program for quad rendering over the camera*/
	vp = new GLProgram(this->m_ClassName + "-Base", GL_VERTEX_SHADER);

	/* Charging a custom per pixel program */
	perPixelProgram = new GLProgram(this->m_ClassName + "-PerPixel", GL_FRAGMENT_SHADER);

	//filter = new GLProgram(this->m_ClassName+"-Filter",GL_FRAGMENT_SHADER);
	m_ProgramPipeline->useProgramStage(GL_VERTEX_SHADER_BIT, vp);

	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, perPixelProgram);
	m_ProgramPipeline->link();

	// Adding and testing another fragment shader file named copy-FS.glsl in the effect/Shaders folder
	//copy = new GLProgram(this->m_ClassName + "-copy", GL_FRAGMENT_SHADER);
	//m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, copy);
	//m_ProgramPipeline->link();
		
	FBO_in = perPixelProgram->uniforms()->getGPUsampler("smp_FBO_in");
	FBO_in->Set(0);

	
}
SampleEffect::~SampleEffect()
{

	delete vp;
	delete perPixelProgram;
}


void SampleEffect::apply(GPUFBO *in, GPUFBO *out)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_DEPTH_TEST);
	if (m_ProgramPipeline)
	{
		/* Drawing to the out framebuffer */
		out->enable();

		/* Binding the in framebuffer as a texture */
		in->bindColorTexture(0);

		/* Launching the per pixel program */
		m_ProgramPipeline->bind();
		quad->drawGeometry(GL_TRIANGLES);
		m_ProgramPipeline->release();
		
		in->releaseColorTexture();
		out->disable();


	}
	glEnable(GL_DEPTH_TEST);
	glPopAttrib();



}
