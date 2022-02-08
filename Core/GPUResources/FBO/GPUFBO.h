#ifndef _GPUFBO_
#define _GPUFBO_

#include <iostream>
#include <glad/glad.h>
#include "Engine/Base/Resource.h"
#include "Engine/OpenGL/ModelGL.h"
#include "Engine/Base/Scene.h"
#include "GPUResources/Textures/GPUTexture2D.h"
#include "GPUResources/Textures/GPUTexture2DArray.h"



class Material;
class DisplayResource;
class DisplayLayer;
// TODO : heritage : GPUFBO1D , 1DARRAY, 2D, 2DARRAY, 3D
class GPUFBO : public Resource
{
public:
	GPUFBO(std::string name);
	~GPUFBO();

	bool create(int w=512,int h=512, int numberOfTarget=1,bool useDepth=true,GLint format = GL_RGBA,GLint target = GL_TEXTURE_2D,int layerDepth = 1);
	void destroy();


	void enable();
	void enable(int layer,int attachment = 0);
	void disable();

	void bindLayerToBuffer(int layer, int buffer);

	void bindColorTexture(int channel = 0,int layer = 0);
	
	void releaseColorTexture();
	void bindDepthTexture(int channel = 0);
	void releaseDepthTexture();

	unsigned int getDepthBuffer();
	GPUTexture* getColorTexture(int layer =0);
	GPUTexture* getDepthTexture();
	void setDepthBuffer(GPUTexture* de);
	void drawBuffer(int i);
	void drawBuffers(int i);

	int getWidth();
	int getHeight();

	bool isInitialized();

	int layer_bound;

	bool* bufferIsBound;

	virtual void display(const glm::vec4 & box = glm::vec4(0.0,0.0,1.0,1.0),int layer = 0);
	virtual void displayDepth(const glm::vec4 & box = glm::vec4(0.0,0.0,1.0,1.0));
private:
	int width;
	int height;
	std::string info_text;
	bool use_depth;
	GLuint FBOId;
	bool CheckFramebufferStatus();

	Scene *scene;

	bool initialized;
	DisplayResource *HUD;
	DisplayLayer*	HUDArray;

	GLuint m_Target;

	std::vector<GPUTexture*> colorBuffers;

	GPUTexture2DArray*		colorBufferArray;

	GPUTexture* depthBuffer;

	int oldFB;
};


#endif
