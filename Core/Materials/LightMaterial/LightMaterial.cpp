#include "LightMaterial.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Scene.h"


LightMaterial::LightMaterial(std::string name, const glm::vec4 & c):
	MaterialGL(name,"LightMaterial")
{
	modelViewProj = vp->uniforms()->getGPUmat4("MVP");

	color = fp->uniforms()->getGPUvec4("color");
	color->Set(c);
}
LightMaterial::~LightMaterial()
{

}

void LightMaterial::setColor(glm::vec4 & c)
{
	color->Set(c);
}

void LightMaterial::render(Node *o)
{

	if (m_ProgramPipeline)
	{
		modelViewProj->Set(o->frame()->getTransformMatrix());
		m_ProgramPipeline->bind();
		o->drawGeometry(GL_TRIANGLES);
		m_ProgramPipeline->release();
	}
	
}
void LightMaterial::update(Node *o,const int elapsedtime)
{
	

}

