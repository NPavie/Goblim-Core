#include "BoundingBoxMaterial.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Scene.h"


BoundingBoxMaterial::BoundingBoxMaterial(std::string name):
	MaterialGL(name,"BoundingBoxMaterial")
{
	modelViewProj = vp->uniforms()->getGPUmat4("MVP");
	modelM = vp->uniforms()->getGPUmat4("Model");

}
BoundingBoxMaterial::~BoundingBoxMaterial()
{

}



void BoundingBoxMaterial::render(Node *o)
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	if (m_ProgramPipeline)
	{		
		//modelViewProj->Set(o->frame()->getTransformMatrix());
		modelM->Set(o->frame()->getRootMatrix());
		m_ProgramPipeline->bind();
		ModelGL* at = dynamic_cast<ModelGL*> (o->getModel());
		if (at != NULL)
			at->bboxGL->drawGeometry();
		m_ProgramPipeline->release();
	}
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void BoundingBoxMaterial::update(Node* o,const int elapsedTime)
{
	/*
	if (o->frame()->updateNeeded())
	{
		modelM->Set(o->frame()->getRootMatrix());
		modelViewProj->Set(o->frame()->getTransformMatrix());
		o->frame()->setUpdate(false);
	} else 	if (Scene::getInstance()->camera()->needUpdate())
	{
		modelViewProj->Set(o->frame()->getTransformMatrix());
		
	}
	*/

	
}