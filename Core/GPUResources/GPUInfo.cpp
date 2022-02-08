#include "GPUResources/GPUInfo.h"

static GPUInfo* GPUInfoGatherer = NULL;

GPUInfo* GPUInfo::Instance()
{
	if (GPUInfoGatherer == NULL)
	{
		GPUInfoGatherer = new GPUInfo();
	}

	return GPUInfoGatherer;
}

GPUInfo::GPUInfo()
{
	OpenGLVersion = NULL;
	GPUName = NULL;
	maximumDrawBuffers = NULL;
	maximumColorAttachments = NULL;
	listOfColorAttachments = NULL;
	maximumSSBO = NULL;
}

GPUInfo::~GPUInfo()
{
	if (OpenGLVersion != NULL) delete OpenGLVersion;
	if (GPUName != NULL) delete GPUName;
	if (maximumDrawBuffers != NULL) delete maximumDrawBuffers;
	if (maximumColorAttachments != NULL) delete maximumColorAttachments;
	if (listOfColorAttachments != NULL) delete[] listOfColorAttachments;
	if (maximumSSBO != NULL) delete maximumSSBO;

}


// Infos de la carte 3D
std::string GPUInfo::getOpenGLVersion()
{
	if (GPUInfoGatherer->OpenGLVersion == NULL)
		GPUInfoGatherer->OpenGLVersion = new std::string( (char*)glGetString(GL_VERSION) );
	
	return *(GPUInfoGatherer->OpenGLVersion);
}

std::string GPUInfo::getGPUName()
{
	if (GPUInfoGatherer->GPUName == NULL)
		GPUInfoGatherer->GPUName = new std::string((char*)glGetString(GL_RENDERER));

	return *(GPUInfoGatherer->GPUName);
}

// Infos pour les FBO

// Return the maximum number of buffer that can be used for silmutaneous rendering
int GPUInfo::getMaximumDrawBuffers()
{
	if (GPUInfoGatherer->maximumDrawBuffers == NULL)
	{
		GPUInfoGatherer->maximumDrawBuffers = new int;
		glGetIntegerv(GL_MAX_DRAW_BUFFERS, GPUInfoGatherer->maximumDrawBuffers);
	}
	return *(GPUInfoGatherer->maximumDrawBuffers);
}

// Return the maximum number of color attachments available for FrameBuffer Objects
int GPUInfo::getMaximumColorAttachments()
{
	if (GPUInfoGatherer->maximumColorAttachments == NULL)
	{
		GPUInfoGatherer->maximumColorAttachments = new int;
		glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, GPUInfoGatherer->maximumColorAttachments);
	}
	return *(GPUInfoGatherer->maximumColorAttachments);
}

GLenum* GPUInfo::getListOfColorAttachments()
{
	if (GPUInfoGatherer->listOfColorAttachments == NULL)
	{
		GPUInfoGatherer->listOfColorAttachments = new GLenum[GPUInfoGatherer->getMaximumColorAttachments()];
		for (int i = 0; i < GPUInfoGatherer->getMaximumColorAttachments(); i++)
		{
			GPUInfoGatherer->listOfColorAttachments[i] = GL_COLOR_ATTACHMENT0 + i;
		}
	}
	return GPUInfoGatherer->listOfColorAttachments;
}

int GPUInfo::getMaximumSSBO()
{
	if (GPUInfoGatherer->maximumSSBO == NULL)
	{
		GPUInfoGatherer->maximumSSBO = new int;
		glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, GPUInfoGatherer->maximumSSBO);
	}
	return *(GPUInfoGatherer->maximumSSBO);
}