#include "Materials/DepthMaterial/DepthMaterial.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Scene.h"
#include <glm/glm.hpp>

DepthMaterial::DepthMaterial(std::string name):
	MaterialGL(name,"DepthMaterial")
{
	CPU_MVP = vp->uniforms()->getGPUmat4("CPU_MVP");
	CPU_ModelView = vp->uniforms()->getGPUmat4("CPU_ModelView");

}
DepthMaterial::~DepthMaterial()
{

}

void DepthMaterial::render(Node *o)
{
	if (m_ProgramPipeline /*|| o->getModel() != NULL*/)
	{	
		// Update matrices if needed
		update(o, 0);

		m_ProgramPipeline->bind();
		//if (o->getLowResModel() != NULL)
		//	o->getLowResModel()->drawGeometry(GL_TRIANGLES);
		//else
			o->drawGeometry(GL_TRIANGLES);
		m_ProgramPipeline->release();
	}
}

void DepthMaterial::update(Node* o,const int elapsedTime)
{
	if (o->frame()->updateNeeded() || Scene::getInstance()->camera()->needUpdate())
	{
		CPU_MVP->Set(o->frame()->getTransformMatrix());
		CPU_ModelView->Set(Scene::getInstance()->camera()->getModelViewMatrix(o->frame()));
	}

	
}