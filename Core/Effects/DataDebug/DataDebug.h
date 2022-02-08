#ifndef _GOBLIM_DATADEBUG_EFFECT_
#define _GOBLIM_DATADEBUG_EFFECT_


#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>


class DataDebug : public EffectGL
{
	public:
        /**
         @brief constructor
         **/
		DataDebug(std::string name);
		~DataDebug();

		virtual void display(const glm::vec4 & box = glm::vec4(0.0,0.0,0.25,0.25));


	protected:
		GLProgram *fp;
		GLProgram *vp;

		GPUvec4* displaybox;
		GPUimage* res;
};
#endif
