#include "Effects/ShellRayCompute/ShellRayCompute.h"
#include "Engine/Base/Node.h"

#include <glm/glm.hpp>
#include "Engine/Base/Scene.h"


ShellRayCompute::ShellRayCompute(std::string name) :
EffectGL(name, "ShellRayCompute")
{
	try{
		// Programs loading
		vp_basic = new GLProgram(this->m_ClassName + "-basic", GL_VERTEX_SHADER);
		vp_extruder = new GLProgram(this->m_ClassName + "-extruder", GL_VERTEX_SHADER);
		gp_borderCloser = new GLProgram(this->m_ClassName + "-borderCloser", GL_GEOMETRY_SHADER);
		fp_shellRayCompute = new GLProgram(this->m_ClassName + "-shellRayCompute", GL_FRAGMENT_SHADER);

		// pipelines creation
		surfaceRenderer = new GLProgramPipeline(this->m_ClassName + "-surfaceRenderer");
		surfaceRenderer->useProgramStage(GL_VERTEX_SHADER_BIT, vp_basic);
		surfaceRenderer->useProgramStage(GL_FRAGMENT_SHADER_BIT, fp_shellRayCompute);
		surfaceRenderer->link();

		shellRenderer = new GLProgramPipeline(this->m_ClassName + "-shellRenderer");
		shellRenderer->useProgramStage(GL_VERTEX_SHADER_BIT, vp_extruder);
		shellRenderer->useProgramStage(GL_GEOMETRY_SHADER_BIT, gp_borderCloser);
		shellRenderer->useProgramStage(GL_FRAGMENT_SHADER_BIT, fp_shellRayCompute);
		shellRenderer->link();

		// Retrieve GPU variables locations
		vp_basic_objectToScreen = vp_basic->uniforms()->getGPUmat4("objectToScreen");
		
		vp_extruder_shellHeight = vp_extruder->uniforms()->getGPUfloat("shellHeight");
		gp_borderCloser_objectToScreen = gp_borderCloser->uniforms()->getGPUmat4("objectToScreen");
		//vp_borderCloser_objectToScreen = vp_extruder->uniforms()->getGPUmat4("objectToScreen");


		fp_shellRayCompute_objectToCamera = fp_shellRayCompute->uniforms()->getGPUmat4("objectToCamera");

		//default initialisation :
		vp_extruder_shellHeight->Set(0.1);

		// other variables initialisation
		currentCamera = Scene::getInstance()->camera();

	}
	catch (const std::exception & e)
	{
		throw logic_error(string("ERROR : Effect ") + this->m_ClassName + string(" : ") + this->m_Name + string(" : ") + e.what() + string("\n"));
	}

}

ShellRayCompute::~ShellRayCompute()
{

}

void ShellRayCompute::renderSurface(Node *o)
{
	if (surfaceRenderer)
	{

		// Backup of current context states
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glEnable(GL_CULL_FACE); 
		glCullFace(GL_BACK);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		vp_basic_objectToScreen->Set(o->frame()->getTransformMatrix());
		fp_shellRayCompute_objectToCamera->Set(currentCamera->getModelViewMatrix(o->frame()));

		surfaceRenderer->bind();
		o->drawGeometry(GL_TRIANGLES);
		surfaceRenderer->release();

		glPopAttrib();
	}
}

void ShellRayCompute::renderSurfaceBack(Node* o)
{
	// Backup of current context states
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	vp_basic_objectToScreen->Set(o->frame()->getTransformMatrix());
	fp_shellRayCompute_objectToCamera->Set(currentCamera->getModelViewMatrix(o->frame()));

	surfaceRenderer->bind();
	o->drawGeometry(GL_TRIANGLES);
	surfaceRenderer->release();

	glPopAttrib();
}

void ShellRayCompute::renderShellBack(Node *o)
{
	if (shellRenderer)
	{

		// Backup of current context states
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		// Culling, depth test and alpha test are required
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_FRONT);	// cull front face
		//glDisable(GL_CULL_FACE);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_GREATER); // keep only the furthest fragment

		glDisable(GL_ALPHA_TEST);
		
		glClearDepth(0.0);	// Background is at depth 0
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		gp_borderCloser_objectToScreen->Set(o->frame()->getTransformMatrix());
		fp_shellRayCompute_objectToCamera->Set(currentCamera->getModelViewMatrix(o->frame()));

		shellRenderer->bind();
		o->drawGeometry(GL_TRIANGLES);
		shellRenderer->release();

		glPopAttrib();
	}
}


void ShellRayCompute::renderShellFront(Node *o)
{
	if (shellRenderer)
	{

		// Backup of current context states
		glPushAttrib(GL_ALL_ATTRIB_BITS);

		// Culling, depth test and alpha test are required
		glPushAttrib(GL_ALL_ATTRIB_BITS);
		glClearColor(0.0, 0.0, 0.0, 0.0);
		//glDisable(GL_CULL_FACE);
		//glEnable(GL_CULL_FACE);
		//glCullFace(GL_BACK);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glClearDepth(1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		gp_borderCloser_objectToScreen->Set(o->frame()->getTransformMatrix());
		fp_shellRayCompute_objectToCamera->Set(currentCamera->getModelViewMatrix(o->frame()));

		shellRenderer->bind();
		o->drawGeometry(GL_TRIANGLES);
		shellRenderer->release();

		glPopAttrib();
	}
}




void ShellRayCompute::setShellHeight(float height){
	vp_extruder_shellHeight->Set(height);
}
