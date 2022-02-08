#ifndef _GOBLIM_BILATERAL_FILTER_
#define _GOBLIM_BILATERAL_FILTER_


#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>
#include "Engine/OpenGL/ModelGL.h"

class BilateralFilter : public EffectGL
{
	public:
		BilateralFilter(std::string name,int blursize = 9,int size = 512);
		~BilateralFilter();

		virtual void apply(GPUFBO *fbo);

	
	protected:
		GPUFBO *temp;
		int fboSize;
		int blurSize;
		
		GPUfloat *texSizeH,*texSizeV;
		GPUsampler *texSamplerH,*texSamplerV;
		GLProgram *blurVertical;
		GLProgram *blurHorizontal;
		GLProgram *vp;
		
		

};
#endif
