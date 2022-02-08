#include "Engine/OpenGL/EffectGL.h"
#include "Engine/Base/Scene.h"

EffectGL::EffectGL(string name,string className):
	Effect(name,className),m_ProgramPipeline(NULL)
{


	GLProgram::prgMgr.addPath(ressourceEffectPath + this->m_ClassName,this->m_ClassName);
	m_ProgramPipeline = new GLProgramPipeline(this->m_ClassName);
	quad = Scene::getInstance()->getModel<ModelGL>(ressourceObjPath+"Quad.obj");
}
EffectGL::~EffectGL()
{
	Scene::getInstance()->releaseModel(quad);
	if (m_ProgramPipeline != NULL)
		delete m_ProgramPipeline;
}
void EffectGL::apply(GPUFBO *fbo)
{
	
}
