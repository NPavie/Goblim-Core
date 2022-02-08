#include "Effects/BendGrass/BendGrass.h"
#include "Engine/OpenGL/EngineGL.h"

BendGrass::BendGrass(std::string name)
	: EffectGL(name,"BendGrass")
{
	vp = new GLProgram(this->m_ClassName+"-Base",GL_VERTEX_SHADER);
	fp = new GLProgram(this->m_ClassName + "-BendGrass", GL_FRAGMENT_SHADER);

	m_ProgramPipeline->useProgramStage(GL_VERTEX_SHADER_BIT, vp);
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, fp);
	m_ProgramPipeline->link();
		

	bendTexture = new GPUFBO(name + "-textureStorage");
	bendTexture->create(1024, 1024, 1, false, GL_RGBA16F, GL_TEXTURE_2D);

	groundData = fp->uniforms()->getGPUsampler("groundData");
	obj_pos = fp->uniforms()->getGPUvec4("obj_pos");

	GPUsampler *p = fp->uniforms()->getGPUsampler("Previous"); 
	p->Set(1);
	radius = fp->uniforms()->getGPUfloat("radius");
	

}

void BendGrass::apply(TextureGenerator *surfaceData, const glm::vec4 & pos,float _radius)
{
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glDisable(GL_DEPTH_TEST);
	if(m_ProgramPipeline )
	{
		
		bendTexture->enable();
		bendTexture->bindColorTexture(1, 0);
		surfaceData->bindTextureArray(groundData->getValue());
		radius->Set(_radius);
		obj_pos->Set(pos);
		// -- Draw call
		m_ProgramPipeline->bind();
		quad->drawGeometry(GL_TRIANGLES);
		m_ProgramPipeline->release();
		bendTexture->releaseColorTexture(),
		
		surfaceData->releaseTextureArray();
		// End
		bendTexture->disable();
		

		
	}
	glPopAttrib();


}


void BendGrass::loadMenu()
{


}

void BendGrass::reload()
{
	try{
		m_ProgramPipeline->release();

		vp = new GLProgram(this->m_ClassName + "-Base", GL_VERTEX_SHADER);
		fp = new GLProgram(this->m_ClassName + "-BendGrass", GL_FRAGMENT_SHADER);

		m_ProgramPipeline->useProgramStage(GL_VERTEX_SHADER_BIT, vp);
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, fp);
		m_ProgramPipeline->link();
		
	}
	catch (const exception & e)
	{
		throw e;
	}
}
