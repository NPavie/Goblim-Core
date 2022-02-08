#ifndef TONE_MAPPING_EFFECT_H_
#define TONE_MAPPING_EFFECT_H_

#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>
#include "Engine/OpenGL/ModelGL.h"


class ToneMappingEffect : public EffectGL {
public:
	ToneMappingEffect (std::string name, bool temporal, float exposure);
	~ToneMappingEffect ();

	void apply (GPUFBO* in, GPUFBO* out);

	inline
	void draw ()
	{
		m_ProgramPipeline->bind ();
		quad->drawGeometry (GL_TRIANGLES);
		m_ProgramPipeline->release ();
	}

	void setExposure (float exposure) {
		tmExposure->Set (exposure);
	}
protected:

	GLProgram *vp;

	GLProgram *TM_PRG_Luminance_Computation;	// Luminance computation Pass (Compute each pixel luminance)
	GLProgram *TM_PRG_Luminance_Downscale;		// Luminance downscale Pass (Compute image max luminance)
	GLProgram *TM_PRG_ToneMapping;				// Tone Mapping (Linearize the scene brightness)

	GPUFBO *TM_FBO_Luminance;
	GPUFBO *TM_FBO_Luminance_Downscale[6];
	GPUFBO *TM_FBO_ToneMapping;

	GPUsampler *TM_Sampler_Luminance_Computation_HDRLuminance;
	GPUsampler *TM_Sampler_Luminance_Downscale_Luminance;
	GPUsampler *TM_Sampler_ToneMapping_HDR;

	GPUfloat *tmMaxRGBValue;
	GPUfloat *tmExposure;
	GPUfloat *tmLuminanceHStep;
	GPUfloat *tmLuminanceVStep;

	//----- CPU Parameters -----//
	int tmNumDownscaleBuffers;		// Tone Mapping : Number of Buffers used for Luminance Downscale
	float tmWaitTime;				// Tone Mapping : Luminance computation time interval
	float tmLastTime;				// Tone Mapping : Last luminance computation time
	float tmTime;					// Tone Mapping : Current time
	float tmLastMaxLuminance;		// Tone Mapping : Last maximum luminance
	float tmLastPower, tmNewPower;	// Tone Mapping : Powers used in the weighted average new luminance
	float data[4096];				// Tone Mapping : Buffer to store 32x32 luminance texture

};

#endif