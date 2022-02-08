/*
	EngineGL overloaded for custom rendering
*/
#include "SampleEngine.h"
#include "Engine/OpenGL/GLProgram.h"
#include "Engine/OpenGL/SceneLoaderGL.h"
#include "Engine/Base/NodeCollectors/MeshNodeCollector.h"
#include "Engine/Base/NodeCollectors/FCCollector.h"
#include "GPUResources/GPUInfo.h"


//#include "Effects/DataDebug/DataDebug.h"
//DataDebug* bufferTester = new DataDebug("LightDebug");

SampleEngine::SampleEngine(int width, int height):
EngineGL(width,height)
{

}
SampleEngine::~SampleEngine()
{
	
}

bool SampleEngine::init()
{
	// ----------------------------
	// Copie de EngineGL::init()
	// ---------------------------

	LOG(INFO) << "Initializing Scene";
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
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_DEPTH_TEST);


	// Force window Resize
	this->onWindowResize(w_Width, w_Height);

    //sampleMaterialForTest = new SampleMaterial("ColorTest");
    
    
	// Bind the light buffer for rendering
	//lightingModel->bind();

	allNodes->collect(scene->getRoot());

	renderedNodes->collect(scene->getRoot());

	for (unsigned int i = 0; i < allNodes->nodes.size(); i++)
		allNodes->nodes[i]->animate(0);
    
    
    lightingModel->update();
    
	return(true);
}

void SampleEngine::render()
{
	// Begin Time query
	timeQuery->begin();

	// Clear Buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Rendering every collected node
	for (unsigned int i = 0; i < renderedNodes->nodes.size(); i++)
		renderedNodes->nodes[i]->render();
				
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

void SampleEngine::animate(const int elapsedTime)
{
	// TODO : reprendre le EngineGL comme exemple
	this->EngineGL::animate(elapsedTime);
}

void SampleEngine::requestUpdate()
{
	
}