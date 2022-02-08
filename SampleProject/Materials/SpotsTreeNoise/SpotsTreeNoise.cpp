#include "SpotsTreeNoise.h"



SpotsTreeNoise::SpotsTreeNoise(std::string name, GPUSpotsTree* treeToRender, GPUFBO* accumulatorFBO, GPUTexture* depthTex)
	: MaterialGL(name, "SpotsTreeNoise")
{
	

	this->spotsTree = treeToRender;

	// Retrieve uniforms
	vp_Model = vp->uniforms()->getGPUmat4("Model");

	this->spotsTree->loadToGPU();
	// Bind bloc of all spots tree buffer :
	int i = 2;
	GPUBuffer::linkBindingPointToProgramStorageBlock(fp, "gaussianDataBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(fp, "harmonicDataBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(fp, "constantDataBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(fp, "distribDataBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(fp, "spotDataBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(fp, "spotsTreeNodeDataBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(fp, "spotIndexBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(fp, "spotsTreeNodeIndexBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(fp, "sizeOfArraysBuffer", ++i);
	
}


SpotsTreeNoise::~SpotsTreeNoise()
{

}

void SpotsTreeNoise::render(Node *o)
{
	if (m_ProgramPipeline)
	{
		
		m_ProgramPipeline->bind();
		o->drawGeometry(GL_TRIANGLES);
		m_ProgramPipeline->release();

	}
}

void SpotsTreeNoise::update(Node *o, const int elapsedTime)
{
	if (Scene::getInstance()->camera()->needUpdate() || o->frame()->updateNeeded())
	{
		vp_Model->Set(o->frame()->getRootMatrix());
	}
}