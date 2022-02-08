#include "PhongMaterial.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Engine.h"


PhongMaterial::PhongMaterial(std::string name,glm::vec3 color):
	MaterialGL(name,"PhongMaterial")
{
	modelViewProj = vp->uniforms()->getGPUmat4("MVP");
	modelView = vp->uniforms()->getGPUmat4("MV");

	camPos = vp->uniforms()->getGPUvec3("camPos");
	lightColor = vp->uniforms()->getGPUvec3("lightPos");

	modelViewF = fp->uniforms()->getGPUmat4("NormalMV");
		
	this->color = fp->uniforms()->getGPUvec3("globalColor");
	this->color->Set(color);

	lightColor->Set(glm::vec3(1.0,1.0,1.0));	
}
PhongMaterial::~PhongMaterial()
{

}

void PhongMaterial::render(Node *o)
{
	camPos->Set(Scene::getInstance()->camera()->convertPtTo(glm::vec3(0.0,0.0,0.0),Scene::getInstance()->frame()));
	lightColor->Set(Scene::getInstance()->getNode("Light")->frame()->convertPtTo(glm::vec3(0.0,0.0,0.0),o->frame()));
	if (m_ProgramPipeline)
	{
		
		m_ProgramPipeline->bind();
			
		modelViewProj->Set(o->frame()->getTransformMatrix());
		modelView->Set(o->frame()->getRootMatrix());
		modelViewF->Set(glm::transpose(glm::inverse(o->frame()->getRootMatrix())));
		
		o->drawGeometry(GL_TRIANGLES);
		m_ProgramPipeline->release();

	}
}

