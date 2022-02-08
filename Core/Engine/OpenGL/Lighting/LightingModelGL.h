#ifndef _LIGHTING_MODEL_GL
#define _LIGHTING_MODEL_GL
#include <GL/glew.h>
#include <vector>
#include "Engine/Base/Lighting/LightingModel.h"
#include "GPUResources/Buffers/GPUBuffer.h"
#include "GPUResources/FBO/GPUFBO.h"

#define LIGHTING_SSBO_BINDING 2

struct LightBuffer
{
	glm::vec4 camPos;
	glm::ivec4 nbLights;
	Light Lights[];
};

// Pour debug
void printLightBuffer(LightBuffer* p);

class LightingModelGL : public LightingModel
{


public:
	LightingModelGL(std::string name);

	LightingModelGL(std::string name, Node* root);

	~LightingModelGL();

	void update(bool forceUpdate = false);
	GPUBuffer* getBuffer();
	void setWindowSize(glm::vec2 size);
		
	void bind(int location = -1);
	void release();

	void renderLights();


private:
	GPUBuffer *m_Buffer;
	GLuint m_BindingPoint;

	GLuint buf;
	glm::vec2 wSize;

	LightBuffer* lightParams;
	int lightParamsSize;
	
};



#endif