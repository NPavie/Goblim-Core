#ifndef GPU_INFOS_H
#define GPU_INFOS_H

#include <iostream>
#include <GL/glew.h>
#include <string>

// Class singleton simple pour la collecte des information du GPU
class GPUInfo
{
public:
	static GPUInfo* Instance();

	// Infos de la carte 3D
	std::string getOpenGLVersion();
	std::string getGPUName();

	// Infos pour les FBO

	// Return the maximum number of buffer that can be used for silmutaneous rendering
	int getMaximumDrawBuffers();

	// Return the maximum number of color attachments available for FrameBuffer Objects
	int getMaximumColorAttachments();

	// Return an array that contains the list of available color attachments point (from 0 to max-1) for layered rendering or MRT
	GLenum* getListOfColorAttachments();

private:
	GPUInfo();
	~GPUInfo();
	
	// GPU infos
	std::string*	OpenGLVersion;
	std::string*	GPUName;
	int*			maximumDrawBuffers;
	int*			maximumColorAttachments;
	GLenum*			listOfColorAttachments;

};


#endif