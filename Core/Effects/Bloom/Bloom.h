#ifndef _GOBLIM_BLOOM_EFFECT_
#define _GOBLIM_BLOOM_EFFECT_

#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>
#include "Engine/OpenGL/ModelGL.h"
#include "Effects/BilateralFilter/BilateralFilter.h"
class Bloom : public EffectGL
{
	public:
		Bloom(std::string name,int finalsize = 512,float bloomTreshold =0.8);
		~Bloom();

		virtual void apply(GPUFBO *in,GPUFBO *out);

		void setBloomTreshold(float v);

	
	protected:
		GPUFBO *temp;
		GPUFBO* bBuffers[4];
		int fboSize;
		int numBuffers;

		BilateralFilter* blur[4];
		
		GPUfloat *bloomTreshold;
		GPUsampler *bloomSampler,*bloomSampler1,*bloomSampler2,*bloomSampler3,*fboIn;
		GLProgram *bloom,*copy,*pass;
		GLProgram *vp;
		float m_bloomTreshold ;
		
		

};
#endif
