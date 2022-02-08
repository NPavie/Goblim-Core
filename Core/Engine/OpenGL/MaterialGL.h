#ifndef _MATERIALGL_
#define _MATERIALGL_

#include "Engine/Base/Material.h"
#include "Engine/Base/Resource.h"
#include "Engine/Base/Resource_mgr.hpp"
#include "Engine/OpenGL/GLProgramPipeline.h"
class MaterialGL : public Material
{
	public :
		MaterialGL(string name,string className);
		~MaterialGL();

		virtual void render(Node *o);
		virtual void update(Node *o, const int elapsedTime){}

		GLProgramPipeline* m_ProgramPipeline;
	protected :
		// Vertex / Geometry / Tessel Control / Tessel Eval / Fragment
		GLProgram *vp, *gp, *tcp, *tep, *fp;
};

#endif