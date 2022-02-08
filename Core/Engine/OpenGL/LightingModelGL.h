#ifndef _LIGHTING_MODEL_GL
#define _LIGHTING_MODEL_GL
#include <glad/glad.h>
#include <vector>
#include "Engine/Base/Lighting/LightingModel.h"
#include "GPUResources/Buffers/GPUBuffer.h"
#include "GPUResources/FBO/GPUFBO.h"

#define LIGHTING_SSBO_BINDING 2 // for SSBO in 4.4


//! @class LightBuffer
//! @brief Buffer of lights to be transfered on GPU
class LightBuffer
{
public:
    //! @brief Constructor for allocation purpose
    //! @param lightsNumber define the number of light to reserve in memory
    LightBuffer(int lightsNumber = 1);
    ~LightBuffer();
    
    struct GPUData
    {
        glm::vec4 camPos;
        glm::ivec4 nbLights;
    }data;
    
    Light* LightsArray;
    
};

struct GPULightBuffer
{
    glm::vec4 campPos;
    glm::ivec4 nbLights;
    Light Lights[64];
};

// Pour debug
void printLightBuffer(GPULightBuffer* p);

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