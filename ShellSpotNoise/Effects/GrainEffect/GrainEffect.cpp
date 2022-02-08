#include "GrainEffect.h"
#include <omp.h>
GrainEffect::GrainEffect (std::string name) : EffectGL (name, "GrainEffect") {
	/* Default Vertex program for quad rendering over the camera*/
	vp = new GLProgram (this->m_ClassName + "-Base", GL_VERTEX_SHADER);
	m_ProgramPipeline->useProgramStage (GL_VERTEX_SHADER_BIT, vp);


	GrainFS = new GLProgram (this->m_ClassName + "-Grain", GL_FRAGMENT_SHADER);

	m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, GrainFS);
	m_ProgramPipeline->link ();

	Grain_Color = GrainFS->uniforms ()->getGPUsampler ("ColorSampler");
	Grain_Color->Set (0);

	time = GrainFS->uniforms ()->getGPUfloat ("time");
	time->Set (0.0f);

	resolution = GrainFS->uniforms ()->getGPUvec2 ("resolution");
	resolution->Set (glm::vec2 (FBO_WIDTH, FBO_HEIGHT));
}

void GrainEffect::apply (GPUFBO* in, GPUFBO* out) {
	m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, GrainFS);


	time->Set (omp_get_wtime ());
	out->enable ();
	in->getColorTexture (0)->bind (Grain_Color->getValue ());
	m_ProgramPipeline->bind ();
	quad->drawGeometry (GL_TRIANGLES);
	m_ProgramPipeline->release ();
	in->getColorTexture (0)->release ();
	out->disable ();
}