#ifndef _GOBLIM_LIGHTSCATTERING_EFFECT_
#define _GOBLIM_LIGHTSCATTERING_EFFECT_

#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>

class LightScattering : public EffectGL
{
	public:
		LightScattering(std::string name,int size = 512,float exposure = 0.005 ,float decay = 0.01,float density = 0.85,float weight = 5.0);
		~LightScattering();

		virtual void apply(GPUFBO *fbo,glm::vec2 lightPos);

		void setDensity(float v);
		void setExposure(float v);
		void setDecay(float v);
		void setWeight(float v);

		


	protected:
		GLProgram *pass,*lightScatter;
		GLProgram *vp;

		GPUfloat *exposure,*decay,*density,*weight;
		GPUvec2 *lightScreenPos;
		GPUsampler *lightBuffer,*fboIn;
		GPUFBO *lightScatterBuffer;

};
#endif
