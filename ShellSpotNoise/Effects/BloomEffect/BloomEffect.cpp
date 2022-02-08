#include "BloomEffect.h"


BloomEffect::BloomEffect (std::string name, float threshold, int blurSize) : EffectGL (name, "BloomEffect") {
	/* Default Vertex program for quad rendering over the camera*/
	vp = new GLProgram (this->m_ClassName + "-Base", GL_VERTEX_SHADER);
	m_ProgramPipeline->useProgramStage (GL_VERTEX_SHADER_BIT, vp);


	Bl_PRG_Bright_Computation = new GLProgram (this->m_ClassName + "-BL-Bright-Computation", GL_FRAGMENT_SHADER);
	Bl_PRG_Bright_HBlur = new GLProgram (this->m_ClassName + "-BL-Bright-HBlur", GL_FRAGMENT_SHADER);
	Bl_PRG_Bright_VBlur = new GLProgram (this->m_ClassName + "-BL-Bright-VBlur", GL_FRAGMENT_SHADER);
	Bl_PRG_Bloom = new GLProgram (this->m_ClassName + "-BL-Bloom", GL_FRAGMENT_SHADER);

	m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, Bl_PRG_Bright_Computation);
	m_ProgramPipeline->link ();
	m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, Bl_PRG_Bright_HBlur);
	m_ProgramPipeline->link ();
	m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, Bl_PRG_Bright_VBlur);
	m_ProgramPipeline->link ();
	m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, Bl_PRG_Bloom);
	m_ProgramPipeline->link ();

	Bl_FBO_Bright = new GPUFBO ("Bl_FBO_Bright");
	Bl_FBO_Bright_Blur[0] = new GPUFBO ("Bl_FBO_Bright_HBlur");
	Bl_FBO_Bright_Blur[1] = new GPUFBO ("Bl_FBO_Bright_VBlur");
	Bl_FBO_Bloom = new GPUFBO ("Bl_FBO_Bloom");

	Bl_FBO_Bright->create (FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
	Bl_FBO_Bright_Blur[0]->create (FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
	Bl_FBO_Bright_Blur[1]->create (FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
	Bl_FBO_Bloom->create (FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);

	Bl_Sampler_Bright_Computation_Color = Bl_PRG_Bright_Computation->uniforms ()->getGPUsampler ("ColorSampler");
	Bl_Sampler_Bright_HBlur_Bright = Bl_PRG_Bright_HBlur->uniforms ()->getGPUsampler ("BrightSampler");
	Bl_Sampler_Bright_VBlur_Bright = Bl_PRG_Bright_VBlur->uniforms ()->getGPUsampler ("BrightSampler");
	Bl_Sampler_Bloom_Color = Bl_PRG_Bloom->uniforms ()->getGPUsampler ("ColorSampler");
	Bl_Sampler_Bloom_Blur = Bl_PRG_Bloom->uniforms ()->getGPUsampler ("BlurSampler");

	Bl_Sampler_Bright_Computation_Color->Set (0);
	Bl_Sampler_Bright_HBlur_Bright->Set (0);
	Bl_Sampler_Bright_VBlur_Bright->Set (0);
	Bl_Sampler_Bloom_Color->Set (0);
	Bl_Sampler_Bloom_Blur->Set (1);

	blThreshold = Bl_PRG_Bright_Computation->uniforms ()->getGPUfloat ("Threshold");
	blHBlurSize = Bl_PRG_Bright_HBlur->uniforms ()->getGPUint ("blBlurSize");
	blHBlurHStep = Bl_PRG_Bright_HBlur->uniforms ()->getGPUfloat ("HStep");
	blVBlurSize = Bl_PRG_Bright_VBlur->uniforms ()->getGPUint ("blBlurSize");
	blVBlurVStep = Bl_PRG_Bright_VBlur->uniforms ()->getGPUfloat ("VStep");

	blThreshold->Set (threshold);
	blHBlurSize->Set (blurSize);
	blHBlurHStep->Set (1.0f / Bl_FBO_Bright_Blur[0]->getWidth ());
	blVBlurSize->Set (blurSize);
	blVBlurVStep->Set (1.0f / Bl_FBO_Bright_Blur[1]->getHeight ());
}

void BloomEffect::apply (GPUFBO* in, GPUFBO* out) {
	//----- Bright Pass -----//
	m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, Bl_PRG_Bright_Computation);

	Bl_FBO_Bright->enable ();
	in->getColorTexture (0)->bind (Bl_Sampler_Bright_Computation_Color->getValue ());
	m_ProgramPipeline->bind ();
	quad->drawGeometry (GL_TRIANGLES);
	m_ProgramPipeline->release ();
	in->getColorTexture (0)->release ();
	Bl_FBO_Bright->disable ();

	//----- Blur Pass -----//
	m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, Bl_PRG_Bright_HBlur);

	Bl_FBO_Bright_Blur[0]->enable ();
	Bl_FBO_Bright->getColorTexture (0)->bind (Bl_Sampler_Bright_HBlur_Bright->getValue ());
	m_ProgramPipeline->bind ();
	quad->drawGeometry (GL_TRIANGLES);
	m_ProgramPipeline->release ();
	Bl_FBO_Bright->getColorTexture (0)->release ();
	Bl_FBO_Bright_Blur[0]->disable ();

	m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, Bl_PRG_Bright_VBlur);

	Bl_FBO_Bright_Blur[1]->enable ();
	Bl_FBO_Bright_Blur[0]->getColorTexture (0)->bind (Bl_Sampler_Bright_VBlur_Bright->getValue ());
	m_ProgramPipeline->bind ();
	quad->drawGeometry (GL_TRIANGLES);
	m_ProgramPipeline->release ();
	Bl_FBO_Bright_Blur[0]->getColorTexture (0)->release ();
	Bl_FBO_Bright_Blur[1]->disable ();

	//----- Bloom Pass -----//
	m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, Bl_PRG_Bloom);

	out->enable ();
	in->getColorTexture (0)->bind (Bl_Sampler_Bloom_Color->getValue ());
	Bl_FBO_Bright_Blur[1]->getColorTexture (0)->bind (Bl_Sampler_Bloom_Blur->getValue ());
	m_ProgramPipeline->bind ();
	quad->drawGeometry (GL_TRIANGLES);
	m_ProgramPipeline->release ();
	Bl_FBO_Bright_Blur[1]->getColorTexture (0)->release ();
	in->getColorTexture (0)->release ();
	out->disable ();
}