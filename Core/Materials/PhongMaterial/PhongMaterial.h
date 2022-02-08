#include "Engine/OpenGL/MaterialGL.h"
#include "GPUResources/Textures/GPUTexture2D.h"
#include <memory.h>

class PhongMaterial : public MaterialGL
{
	public:
		PhongMaterial(std::string name, glm::vec3 color = glm::vec3(0.7,0.7,0.7));
		~PhongMaterial();

		virtual void render(Node *o);

		GPUmat4 *modelViewProj,*modelView,*modelViewF;

		GPUvec3* camPos;
		GPUvec3* lightPos;
		GPUvec3* lightColor;
		GPUvec3* color;

	protected:
		bool loadShaders();
};
