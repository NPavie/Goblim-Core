#include "ToneMappingEffect.h"
#include <omp.h>

ToneMappingEffect::ToneMappingEffect (std::string name, bool temporal, float exposure) : EffectGL (name, "ToneMappingEffect") {
	/* Default Vertex program for quad rendering over the camera*/
	vp = new GLProgram (this->m_ClassName + "-Base", GL_VERTEX_SHADER);
	m_ProgramPipeline->useProgramStage (GL_VERTEX_SHADER_BIT, vp);

	tmNewPower = 0.5f;
	tmLastPower = 0.5f;
	tmLastTime = 0.0f;
	tmLastMaxLuminance = 1.0f;
	if (temporal)
		tmWaitTime = 0.04f; // 0.04f
	else
		tmWaitTime = 0.0f;


	// ---- Programs
	TM_PRG_Luminance_Computation = new GLProgram (this->m_ClassName + "-TM-Luminance-Computation", GL_FRAGMENT_SHADER);
	TM_PRG_Luminance_Downscale = new GLProgram (this->m_ClassName + "-TM-Luminance-Downscale", GL_FRAGMENT_SHADER);
	TM_PRG_ToneMapping = new GLProgram (this->m_ClassName + "-TM-ToneMapping", GL_FRAGMENT_SHADER);

	m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, TM_PRG_Luminance_Computation);
	m_ProgramPipeline->link ();
	m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, TM_PRG_Luminance_Downscale);
	m_ProgramPipeline->link ();
	m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, TM_PRG_ToneMapping);
	m_ProgramPipeline->link ();
	// ----



	// ----  FBOs
	unsigned int FBOWidth, FBOHeight, i;
	i = 0;
	FBOWidth = FBO_WIDTH / 2;
	FBOHeight = FBO_HEIGHT / 2;

	TM_FBO_Luminance = new GPUFBO ("TM_FBO_Luminance");
	TM_FBO_ToneMapping = new GPUFBO ("TM_FBO_Luminance");

	TM_FBO_Luminance->create (FBO_WIDTH, FBO_HEIGHT, 1, false, GL_R16F, GL_TEXTURE_2D, 1);
	TM_FBO_ToneMapping->create (FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGBA16F, GL_TEXTURE_2D, 1);

	TM_FBO_Luminance_Downscale[6];
	do
	{
		TM_FBO_Luminance_Downscale[i] = new GPUFBO ("TM_FBO_Luminance_Downscale");
		TM_FBO_Luminance_Downscale[i]->create (FBOWidth, FBOHeight, 1, false, GL_R16F, GL_TEXTURE_2D, 1);

		FBOWidth = FBOWidth / 2;
		FBOHeight = FBOHeight / 2;

		++i;
	} while (FBOWidth > 32 || FBOHeight > 32);

	tmNumDownscaleBuffers = i;
	// ---- 

	// ---- Samplers 
	TM_Sampler_Luminance_Computation_HDRLuminance = TM_PRG_Luminance_Computation->uniforms ()->getGPUsampler ("HDRLuminanceSampler");
	TM_Sampler_Luminance_Downscale_Luminance = TM_PRG_Luminance_Downscale->uniforms ()->getGPUsampler ("LuminanceSampler");
	TM_Sampler_ToneMapping_HDR = TM_PRG_ToneMapping->uniforms ()->getGPUsampler ("HDRSampler");

	TM_Sampler_Luminance_Computation_HDRLuminance->Set (0);
	TM_Sampler_Luminance_Downscale_Luminance->Set (0);
	TM_Sampler_ToneMapping_HDR->Set (0);
	// ---- 

	// ---- GPUParameters
	tmLuminanceHStep = TM_PRG_Luminance_Downscale->uniforms ()->getGPUfloat ("HStep");
	tmLuminanceVStep = TM_PRG_Luminance_Downscale->uniforms ()->getGPUfloat ("VStep");
	tmMaxRGBValue = TM_PRG_ToneMapping->uniforms ()->getGPUfloat ("tmMaxRGBValue");
	tmExposure = TM_PRG_ToneMapping->uniforms ()->getGPUfloat ("tmExposure");
	tmExposure->Set (exposure);
	// ----



}

void ToneMappingEffect::apply (GPUFBO* in, GPUFBO* out) {
	tmTime = omp_get_wtime ();

	if ((tmTime - tmLastTime) >= tmWaitTime)
	{
		tmLastTime = tmTime;

		//----- Luminance Pass -----//
		m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, TM_PRG_Luminance_Computation);

		TM_FBO_Luminance->enable ();
		in->getColorTexture (0)->bind (TM_Sampler_Luminance_Computation_HDRLuminance->getValue ());
		this->draw ();
		in->getColorTexture (0)->release ();
		TM_FBO_Luminance->disable ();

		//----- Downscale Pass -----//
		m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, TM_PRG_Luminance_Downscale);

		tmLuminanceHStep->Set (1.0f / TM_FBO_Luminance_Downscale[0]->getHeight ());
		tmLuminanceVStep->Set (1.0f / TM_FBO_Luminance_Downscale[0]->getWidth ());
		TM_FBO_Luminance_Downscale[0]->enable ();
		TM_FBO_Luminance->getColorTexture (0)->bind (TM_Sampler_Luminance_Downscale_Luminance->getValue ());
		this->draw ();
		TM_FBO_Luminance->getColorTexture (0)->release ();
		TM_FBO_Luminance_Downscale[0]->disable ();

		for (unsigned int i = 1; i < tmNumDownscaleBuffers; ++i)
		{
			tmLuminanceHStep->Set (1.0f / TM_FBO_Luminance_Downscale[i]->getHeight ());
			tmLuminanceVStep->Set (1.0f / TM_FBO_Luminance_Downscale[i]->getWidth ());
			TM_FBO_Luminance_Downscale[i]->enable ();
			TM_FBO_Luminance_Downscale[i - 1]->getColorTexture (0)->bind (TM_Sampler_Luminance_Downscale_Luminance->getValue ());
			this->draw ();
			TM_FBO_Luminance_Downscale[i - 1]->getColorTexture (0)->release ();
			TM_FBO_Luminance_Downscale[i]->disable ();
		}

		glBindTexture (GL_TEXTURE_2D, TM_FBO_Luminance_Downscale[tmNumDownscaleBuffers - 1]->getColorTexture ()->getId ());
		glGetTexImage (GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, data);
		glBindTexture (GL_TEXTURE_2D, 0);

		float maxLuminance = 0.0f;


		for (int p = 0; p < (TM_FBO_Luminance_Downscale[tmNumDownscaleBuffers - 1]->getHeight () * TM_FBO_Luminance_Downscale[tmNumDownscaleBuffers - 1]->getWidth ()); ++p)
		{
			maxLuminance = (glm::max)(maxLuminance, data[p]);
		}

		//if (maxLuminance < 1.0)
		//	maxLuminance = 1.0;

		const float delta = tmLastMaxLuminance - maxLuminance;

		const float valeur1 = 0.00004; // 0.04
		const float valeur2 = 0.0016; // 0.0016

		if (delta < 0.0f)
		{
			if (tmNewPower > valeur1)
			{
				tmLastPower += valeur1;
				tmNewPower -= valeur1;
			}

			if (tmWaitTime > valeur1)
				tmWaitTime -= valeur2;
		}
		else
		{
			if (tmLastPower > valeur1)
			{
				tmLastPower -= valeur1;
				tmNewPower += valeur1;
			}
			tmWaitTime += valeur2;
		}

		tmLastMaxLuminance = (tmLastMaxLuminance * tmLastPower) + (maxLuminance * tmNewPower);

		if (tmLastMaxLuminance < 1.0f)
			tmLastMaxLuminance = 1.0f;
	}

	//----- ToneMapping Pass -----//
	m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, TM_PRG_ToneMapping);

	tmMaxRGBValue->Set (tmLastMaxLuminance);
	out->enable ();
	in->getColorTexture (0)->bind (TM_Sampler_ToneMapping_HDR->getValue ());
	this->draw ();
	in->getColorTexture (0)->release ();
	out->disable ();
}