#include "ColorMaterial.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Scene.h"


ColorMaterial::ColorMaterial(std::string name, const glm::vec4 & c):
	MaterialGL(name,"ColorMaterial")
{
	
	modelM = vp->uniforms()->getGPUmat4("Model");
	modelViewProj = vp->uniforms()->getGPUmat4("MVP");
	color = fp->uniforms()->getGPUvec4("CPU_color");
	color->Set(c);

}
ColorMaterial::~ColorMaterial()
{

}

void ColorMaterial::setColor(glm::vec4 & c)
{
	color->Set(c);
}

void ColorMaterial::render(Node *o)
{
	if (m_ProgramPipeline)
	{		
		m_ProgramPipeline->bind();
		o->drawGeometry(GL_TRIANGLES);
		m_ProgramPipeline->release();
	}
}

void ColorMaterial::update(Node* o,const int elapsedTime)
{
	
	if (o->frame()->updateNeeded())
	{
		modelM->Set(o->frame()->getRootMatrix());
	} 
	
	
}