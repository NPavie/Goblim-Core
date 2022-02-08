#ifndef _EFFECTGL_
#define _EFFECTGL_

#include "Engine/Base/Effect.h"
#include "Engine/OpenGL/GLProgramPipeline.h"
#include "GPUResources/FBO/GPUFBO.h"
#include "Engine/OpenGL/ModelGL.h"



class EffectGL : public Effect
{
	public :
		EffectGL(string name,string className);
		~EffectGL();

		//virtual void apply();
		virtual void apply(GPUFBO *fbo=NULL);

	protected :
		GLProgramPipeline* m_ProgramPipeline;
		ModelGL *quad;
};

#endif
