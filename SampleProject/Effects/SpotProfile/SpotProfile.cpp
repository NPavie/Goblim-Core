#include "Effects/SpotProfile/SpotProfile.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Engine.h"



SpotProfile::SpotProfile(std::string name, int spotID, bool isSpot, bool isDistribProfile) :
EffectGL(name, "SpotProfile")
{
	/* Default Vertex program for quad rendering over the camera*/
	vp = new GLProgram(this->m_ClassName + "-Base", GL_VERTEX_SHADER);

	/* Charging a custom per pixel program */
	perPixelProgram = new GLProgram(this->m_ClassName + "-PerPixel", GL_FRAGMENT_SHADER);

	//filter = new GLProgram(this->m_ClassName+"-Filter",GL_FRAGMENT_SHADER);
	m_ProgramPipeline->useProgramStage(GL_VERTEX_SHADER_BIT, vp);

	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, perPixelProgram);
	m_ProgramPipeline->link();

	// Bind bloc of all spots tree buffer :
	int i = 2;
	GPUBuffer::linkBindingPointToProgramStorageBlock(perPixelProgram, "gaussianDataBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(perPixelProgram, "harmonicDataBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(perPixelProgram, "constantDataBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(perPixelProgram, "distribDataBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(perPixelProgram, "spotDataBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(perPixelProgram, "spotIndexBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(perPixelProgram, "sizeOfArraysBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(perPixelProgram, "weightsDataBuffer", ++i);

	// Adding and testing another fragment shader file named copy-FS.glsl in the effect/Shaders folder
	//copy = new GLProgram(this->m_ClassName + "-copy", GL_FRAGMENT_SHADER);
	//m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, copy);
	//m_ProgramPipeline->link();
	
	fp_spotID = perPixelProgram->uniforms()->getGPUint("dataID");
	fp_spotID->Set(spotID);

	fp_isDistrib = perPixelProgram->uniforms()->getGPUint("isDistrib");
	fp_isDistrib->Set(isDistribProfile ? 1 : 0);

	fp_isSpot = perPixelProgram->uniforms()->getGPUint("isSpot");
	fp_isSpot->Set(isSpot ? 1 : 0);
	

	FBO_in = perPixelProgram->uniforms()->getGPUsampler("smp_FBO_in");
	FBO_in->Set(0);

	
}
SpotProfile::~SpotProfile()
{

	delete vp;
	delete perPixelProgram;
}


void SpotProfile::apply(GPUFBO *in, GPUFBO *out)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_DEPTH_TEST);
	if (m_ProgramPipeline)
	{
		/* Drawing to the out framebuffer */
		out->enable();

		/* Binding the in framebuffer as a texture */
		if(in!=NULL)in->bindColorTexture(0);

		/* Launching the per pixel program */
		m_ProgramPipeline->bind();
		quad->drawGeometry(GL_TRIANGLES);
		m_ProgramPipeline->release();
		
		if (in != NULL)in->releaseColorTexture();
		out->disable();


	}
	glEnable(GL_DEPTH_TEST);
	glPopAttrib();



}
