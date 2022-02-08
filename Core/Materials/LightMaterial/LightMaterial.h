#ifndef _LIGHT_MATERIAL
#define _LIGHT_MATERIAL

#include "Engine/OpenGL/MaterialGL.h"
#include "GPUResources/FBO/GPUFBO.h"

class LightMaterial : public MaterialGL
{
	public:
		LightMaterial(std::string name, const glm::vec4 & c = glm::vec4(1.0,1.0,1.0,1.0));
		~LightMaterial();
		void setColor(glm::vec4 & c);

		virtual void render(Node *o);
		virtual void update(Node* o,const int elapsedTime = 0);

		GPUmat4* modelViewProj;
		GPUvec4* color;

	protected:
		bool loadShaders();
};
#endif