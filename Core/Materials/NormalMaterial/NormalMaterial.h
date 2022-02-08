#ifndef _NORMAL_MATERIAL
#define _NORMAL_MATERIAL

#include "Engine/OpenGL/MaterialGL.h"

#include <memory.h>

class NormalMaterial : public MaterialGL
{
	public:
		NormalMaterial(std::string name);
		~NormalMaterial();

		virtual void render(Node *o);
		void updateCamera();
		GPUmat4* modelViewProj;
		GPUmat4* modelView;
		GPUmat4* normalModelView;
		GPUmat4* modelMatrix;
		GPUfloat *zFar,*zNear;


	protected:
		bool loadShaders();
};
#endif