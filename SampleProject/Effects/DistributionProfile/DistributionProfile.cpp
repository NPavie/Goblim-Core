#include "Effects/DistributionProfile/DistributionProfile.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Engine.h"



DistributionProfile::DistributionProfile(std::string name, int distribID) :
EffectGL(name, "DistributionProfile")
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

	FBO_in = perPixelProgram->uniforms()->getGPUsampler("smp_FBO_in");
	FBO_in->Set(0);

	fp_distribID = perPixelProgram->uniforms()->getGPUint("distribID");
	fp_distribID->Set(distribID);

	
}
DistributionProfile::~DistributionProfile()
{

	delete vp;
	delete perPixelProgram;
}


void DistributionProfile::apply(GPUFBO *in, GPUFBO *out)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_DEPTH_TEST);
	if (m_ProgramPipeline)
	{
		/* Drawing to the out framebuffer */
		out->enable();

		/* Binding the in framebuffer as a texture */
		if (in != NULL)in->bindColorTexture(0);

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
