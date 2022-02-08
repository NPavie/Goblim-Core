/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */
#include <string>
#include "EngineGL.h"
#include "Engine/OpenGL/Lighting/LightingModelGL.h"
#include "SceneLoaderGL.h"
#include "Engine/Base/NodeCollectors/MeshNodeCollector.h"
#include "Engine/Base/NodeCollectors/FCCollector.h"



EngineGL::EngineGL(int width, int height):
Engine()
{
	this->scene = Scene::getInstance();
	this->allNodes = new StandardCollector();
	this->renderedNodes = new MeshNodeCollector();
	//this->renderedNodes = new FCCollector();
	w_Width = width;
	w_Height = height;
	this->lightingModel = NULL;	
	timeQuery = new GPUQuery("Timer",GL_TIME_ELAPSED);
	scene->needupdate = true;	
	drawLights = false;
	drawBoundingBoxes = false;
}

EngineGL::~EngineGL()
{
	delete allNodes;
	delete renderedNodes;
} 

bool EngineGL::init()
{
	LOG(INFO) << "Initializing Scene" ;
	timeQuery->create();	

	// Load shaders in "\common" into graphic cards
	GLProgram::prgMgr.addPath(ressourceMaterialPath + "Common", "");

	// Loading a scene or a mesh and add it to the root of the Engine scenegraphe
	SceneLoaderGL* sceneloader = new SceneLoaderGL();
	Node* sceneLoaded = sceneloader->loadScene(ressourceObjPath + "Sphere.obj");
	scene->getSceneNode()->adopt(sceneLoaded);

	// Create Lighting Model GL and collect all light nodes of the scene 
	lightingModel = new LightingModelGL("LightingModel", scene->getRoot());
	
	// Create Bounding Box Material for bounding box rendering
	boundingBoxMat = new BoundingBoxMaterial("BoundingBoxMat");

	// OpenGL state variable initialisation
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_DEPTH_TEST);

	
	// Force window Resize
	this->onWindowResize(w_Width, w_Height);
	
	// Bind the light buffer for rendering
	lightingModel->bind();

	allNodes->collect(scene->getRoot());

	renderedNodes->collect(scene->getRoot());

	for (unsigned int i = 0; i < allNodes->nodes.size(); i++)
		allNodes->nodes[i]->animate(0);
	return(true);
}



void EngineGL::render()
{
		// Begin Time query
		timeQuery->begin();

		// Clear Buffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Rendering every collected node
		for (unsigned int i = 0; i < renderedNodes->nodes.size(); i++) 
			renderedNodes->nodes[i]->render();

		// end time Query					
		timeQuery->end();

	
		if (drawLights)
			lightingModel->renderLights();

		if (drawBoundingBoxes)
			for (unsigned int i = 0; i < renderedNodes->nodes.size(); i++) 
				renderedNodes->nodes[i]->render(boundingBoxMat);


		// scene has been rendered : no need for recomputing values until next camera/parameter change
		//Scene::getInstance()->camera()->setUpdate(true);
		scene->needupdate = false;
		
}

void EngineGL::animate(const int elapsedTime)
{

	//dynamic_cast<FCCollector*> (renderedNodes)->updateCam();
	// Collect all relevant nodes
	//renderedNodes->collect(scene->getRoot());
	
	// Animate each node
	for (unsigned int i = 0; i < allNodes->nodes.size(); i++)
		allNodes->nodes[i]->animate(elapsedTime);

	// force update of lighting model
	//Scene::getInstance()->camera()->updateBuffer();
	lightingModel->update(true); 
	
}

void EngineGL::onWindowResize(int width, int height)
{
	
	glViewport(0, 0, width, height);
	scene->camera()->setAspectRatio((float)width / (float)height);
	w_Width = width;
	w_Height = height;

	if (lightingModel != NULL)
		lightingModel->setWindowSize(glm::vec2(width, height));
		
}

void EngineGL::setWidth(int w)
{
	w_Width = w;
}
void EngineGL::setHeight(int h)
{
	w_Height = h;
}

int EngineGL::getWidth()
{
	return w_Width;
}

int EngineGL::getHeight()
{
	return w_Height;
}
double EngineGL::getFrameTime()
{
	double frame_time = timeQuery->getResultUInt();
	//double frame_time = 5;
	frame_time = frame_time*0.000001;
	return frame_time;
}



