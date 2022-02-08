#include "SampleMaterial.h"
#include "Engine/Base/Node.h"



SampleMaterial::SampleMaterial(std::string name) :
MaterialGL(name,"SampleMaterial")
{
	modelViewMatrix = vp->uniforms()->getGPUmat4("CPU_modelViewMatrix");
	
}

SampleMaterial::~SampleMaterial()
{	

}


void SampleMaterial::render(Node *o)
{	
	if (m_ProgramPipeline)
	{
		
		glClearColor(0.0,0.0,0.0,0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		m_ProgramPipeline->bind();
		o->drawGeometry(GL_TRIANGLES);
		m_ProgramPipeline->release();
	}	
}

void SampleMaterial::update(Node* o, const int elapsedTime)
{
	if (o->frame()->updateNeeded())
	{
		modelViewMatrix->Set(o->frame()->getTransformMatrix());
		//o->frame()->setUpdate(false);
	}
	
}