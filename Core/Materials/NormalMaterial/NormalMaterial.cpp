#include "NormalMaterial.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Scene.h"


NormalMaterial::NormalMaterial(std::string name):
	MaterialGL(name,"NormalMaterial")
{
	modelViewProj = vp->uniforms()->getGPUmat4("MVP");
	modelView = vp->uniforms()->getGPUmat4("MV");
	normalModelView = vp->uniforms()->getGPUmat4("NMV");
	modelMatrix = vp->uniforms()->getGPUmat4("ModelMatrix");
	zFar = vp->uniforms()->getGPUfloat("far");	
	zNear = vp->uniforms()->getGPUfloat("near");
	updateCamera();
}
NormalMaterial::~NormalMaterial()
{

}
void NormalMaterial::updateCamera()
{
	zFar->Set(Scene::getInstance()->camera()->getZfar());
	zNear->Set(Scene::getInstance()->camera()->getZnear());
}



void NormalMaterial::render(Node *o)
{
	if (m_ProgramPipeline)
	{
		modelView->Set(Scene::getInstance()->camera()->getModelViewMatrix(o->frame()));
		modelViewProj->Set(o->frame()->getTransformMatrix());
		normalModelView->Set(glm::transpose(glm::inverse(Scene::getInstance()->camera()->getModelViewMatrix(o->frame()))));
		//modelMatrix->Set(glm::transpose(glm::inverse(o->frame()->getRootMatrix())));
		//
		m_ProgramPipeline->bind();
		o->drawGeometry(GL_TRIANGLES);
		m_ProgramPipeline->release();
	}
}

