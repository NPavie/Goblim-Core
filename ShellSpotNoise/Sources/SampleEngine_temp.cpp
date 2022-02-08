/*
	EngineGL overloaded for custom rendering
*/
#include "SampleEngine.h"
#include "Engine/OpenGL/v4/GLProgram.h"
#include "Engine/OpenGL/SceneLoaderGL.h"
#include "Engine/Base/NodeCollectors/MeshNodeCollector.h"
#include "Engine/Base/NodeCollectors/FCCollector.h"
#include "Voxel.h"

#include "Utils/ImageUtilities/ImageUtils.h"

SampleEngine::SampleEngine(int width, int height):
EngineGL(width,height)
{

}

SampleEngine::~SampleEngine()
{
	
}

bool SampleEngine::init()
{
	LOG(INFO) << "Initializing Scene";
	timeQuery->create();

	// Load shaders in "\common" into graphic cards
	GLProgram::prgMgr.addPath(ressourceMaterialPath + "Common", "");

	// Loading a scene or a mesh and add it to the root of the Engine scenegraphe
	SceneLoaderGL* sceneloader = new SceneLoaderGL();
	//Node* sceneLoaded = sceneloader->loadScene(ressourceObjPath + "FurryCoat/CoatAndBody.DAE");
	//scene->getSceneNode()->adopt(sceneLoaded);
	//ground = scene->getNode("Coat");

	Node* sceneLoaded = sceneloader->loadScene(ressourceObjPath + "FurryBunny/Bunny.obj");
	scene->getSceneNode()->adopt(sceneLoaded);
	ground = scene->getNode("Bunny");

	//Node* sceneLoaded = sceneloader->loadScene(ressourceObjPath + "Plane/Plane.DAE");
	//scene->getSceneNode()->adopt(sceneLoaded);
	//ground = scene->getNode("Sol");
	


	kernelsConfiguration = new KernelList();
	//kernelsConfiguration->loadFromFolder(ressourceObjPath + "Plane/GrassInstancing");
	//kernelsConfiguration->loadFromFolder(ressourceObjPath + "Plane/Grass");
	kernelsConfiguration->loadFromFolder(ressourceObjPath + "FurryBunny/FurElements");
	//kernelsConfiguration->loadFromFolder(ressourceObjPath + "Plane_old/Grass");
	kernelsConfiguration->setGridHeight(1.0);
	kernelsConfiguration->prepareGPUData();
	kernelsConfiguration->updateGPUData();
	kernelsConfiguration->bindBuffer(3);
	kernelsConfiguration->loadAnttweakMenu();

	ModelGL* quad = Scene::getInstance()->getModel<ModelGL>(ressourceObjPath + "Quad.obj");

	pierre = Scene::getInstance()->getNode("Pierre");
	pierre->setModel(Scene::getInstance()->getModel<ModelGL>(ressourceObjPath + "Sphere.obj"));
	pierre->setMaterial(new ColorMaterial("PiereMat", glm::vec4(0.7, 0.0, 0.2, 1.0)));
	pierre->frame()->scale(glm::vec3(50.0));
	scene->getSceneNode()->adopt(pierre);
	
	depthRenderer = new GPUFBO("DepthRenderer");
	depthRenderer->create(w_Width, w_Height, 1, true, GL_RGBA32F, GL_TEXTURE_2D, 1);

	mainRenderer = new GPUFBO("MainRenderer");
	mainRenderer->create(w_Width, w_Height, 1, true, GL_RGBA32F, GL_TEXTURE_2D, 1);

	screenSizeFBO = new GPUFBO("secondRenderer");
	screenSizeFBO->create(w_Width, w_Height, 2, true, GL_RGBA32F, GL_TEXTURE_2D, 1);

	grassPostProcess = NULL;
	grassSplatted = NULL;
	grassInstanced = NULL;
	grassPeeled = NULL;
	grassVolumic = NULL;


	//grassInstanced = new InstancedKernelMaterial("InstancedGrass", kernelsConfiguration, screenSizeFBO, mainRenderer->getDepthTexture());
	grassSplatted = new InstancedSplattedKernelMaterial("GrassSplatted", kernelsConfiguration, screenSizeFBO, depthRenderer->getColorTexture(0));
	//grassPeeled = new DepthPeeledShellMaterial("GrassPeeled", kernelsConfiguration, screenSizeFBO, mainRenderer->getDepthTexture());
	//grassPostProcess = new GrassComposer("GrassAdder", kernelsConfiguration, this);
	//grassPostProcess->setComposerFor(ground);
	
	//grassVolumic = new InstancedVolumetricKernelMaterial("GrassVolumic", kernelsConfiguration, screenSizeFBO, depthRenderer->getColorTexture(0));

	GPUTexture2D* solemio = new GPUTexture2D(ressourceObjPath + "Plane/LEDirt01.dds");
	GPUTexture2D* solemio_n = new GPUTexture2D(ressourceObjPath + "Plane/LEDirt01_n.dds");
	solTexture = new TextureMaterial("Soooool", solemio, solemio_n);
	ground->setMaterial(solTexture);

	// TODO : VolumetricShellSpotNoise

	depthCompute = new DepthMaterial("DepthCompute");

	bendGrass = new BendGrass("BendGrass");
	
	// Create Lighting Model GL and collect all light nodes of the scene 
	lightingModel = new LightingModelGL("LightingModel", scene->getRoot());

	// Create Bounding Box Material for bounding box rendering
	boundingBoxMat = new BoundingBoxMaterial("BoundingBoxMat");


	// OpenGL state variable initialisation
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glEnable(GL_DEPTH_TEST);


	

	// Bind the light buffer for rendering
	lightingModel->bind();

	allNodes->collect(scene->getRoot());

	renderedNodes->collect(scene->getRoot());

	for (unsigned int i = 0; i < allNodes->nodes.size(); i++)
		allNodes->nodes[i]->animate(0);
	
	

	kernelsConfiguration->prepareGPUData();
	kernelsConfiguration->updateGPUData();
	kernelsConfiguration->bindBuffer(3);
	kernelsConfiguration->loadAnttweakMenu();

	

	// Force window Resize
	this->onWindowResize(w_Width, w_Height);

	//scene->camera()->setZnear(0.1f);
	//scene->camera()->setZfar(500.0f);

	activeCapture = false;
	frameCounter = 0;
	return(true);
}

void SampleEngine::render()
{
	// Begin Time query
	timeQuery->begin();

	kernelsConfiguration->updateGPUData();
	
	depthRenderer->enable();
	// MANUAL DEPTH COMPUTE ---------------------
	depthRenderer->drawBuffer(0);
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glClearColor(1.0, 1.0, 1.0, 0.0);
	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	for (unsigned int i = 0; i < renderedNodes->nodes.size(); i++)
		renderedNodes->nodes[i]->render(depthCompute);
	// MANUAL DEPTH COMPUTE --------------------- THE END
	glPopAttrib();

	depthRenderer->disable();

	mainRenderer->enable();
	mainRenderer->drawBuffer(0);
	// Clear Buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Rendering every collected node
	for (unsigned int i = 0; i < renderedNodes->nodes.size(); i++)
		renderedNodes->nodes[i]->render();
	
	if (grassInstanced != NULL)
	{
		grassInstanced->render(ground);
	}

	if (grassSplatted != NULL)
	{
		grassSplatted->render(ground);

		glm::vec3 pierre_in_sol = pierre->frame()->convertPtTo(glm::vec3(0.0), ground->frame());
		bendGrass->apply(grassSplatted->surfaceData, glm::vec4(pierre_in_sol, 1.0), 10.0);
	}

	if (grassPeeled != NULL)
	{
		grassPeeled->render(ground);
	}

	if (grassVolumic != NULL)
	{
		grassVolumic->render(ground);
	}


	


	if (drawLights)
		lightingModel->renderLights();

	if (drawBoundingBoxes)
		for (unsigned int i = 0; i < renderedNodes->nodes.size(); i++)
			renderedNodes->nodes[i]->render(boundingBoxMat);
	
	

	
	
	// scene has been rendered : no need for recomputing values until next camera/parameter change
	//Scene::getInstance()->camera()->setUpdate(true);
	mainRenderer->disable();
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	if (grassPostProcess != NULL)
	{
		glClearColor(1.0, 1.0, 1.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		grassPostProcess->timer->Set(frameCounter / 60.0f);
		grassPostProcess->apply(mainRenderer);
	}
	if (grassSplatted != NULL || grassInstanced != NULL || grassPeeled != NULL || grassVolumic != NULL)
	{
		mainRenderer->display();
	}

	//bendGrass->bendTexture->display(glm::vec4(0.0, 0.0, 0.1, 0.1));
	//
	//depthRenderer->display(glm::vec4(0, 0.1, 0.1, 0.1), 0);

	//mainRenderer->display(glm::vec4(0,0,1,1),0);
	// end time Query					
	timeQuery->end();

	if (activeCapture)
	{
		//std::cout << "It works !" << std::endl;
		// Capture the frame
		unsigned char* image = (unsigned char*)malloc(sizeof(unsigned char) * 3 * this->w_Width * this->w_Height);
		glReadPixels(0, 0, this->w_Width, this->w_Height, GL_RGB, GL_UNSIGNED_BYTE, image);
		// Warning : enregistre de bas en haut
				
		// save it in a file
		std::string fileName("./Capture/frame_" + std::to_string(frameCounter) + ".bmp");
		//std::string fileName("last.bmp");
		int res = SOIL_save_image(fileName.c_str(), SOIL_SAVE_TYPE_BMP, this->w_Width, this->w_Height, 3, image);
		
	}
	frameCounter++;

	scene->needupdate = false;
}

void SampleEngine::animate(const int elapsedTime)
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

void SampleEngine::onWindowResize(int w, int h)
{
	EngineGL::onWindowResize(w, h);
	//if (mainRenderer != NULL) delete mainRenderer;
	//mainRenderer = new GPUFBO("MainRenderer");
	//mainRenderer->create(w_Width, w_Height, 1, true, GL_RGBA32F, GL_TEXTURE_2D, 1);

	//if (screenSizeFBO != NULL) delete screenSizeFBO;
	//screenSizeFBO = new GPUFBO("secondRenderer");
	//screenSizeFBO->create(w_Width, w_Height, 1, true, GL_RGBA32F, GL_TEXTURE_2D, 1);

}


void SampleEngine::useInstancing()
{
	grassPostProcess = NULL;
	grassSplatted = NULL;
	grassInstanced = NULL;
	grassPeeled = NULL;
	grassVolumic = NULL;

	delete kernelsConfiguration;
	kernelsConfiguration = new KernelList();
	kernelsConfiguration->loadFromFolder(ressourceObjPath + "Plane/GrassInstancing");
	kernelsConfiguration->setGridHeight(1.0);
	kernelsConfiguration->prepareGPUData();
	kernelsConfiguration->updateGPUData();
	kernelsConfiguration->bindBuffer(3);
	kernelsConfiguration->loadAnttweakMenu();

	grassInstanced = new InstancedKernelMaterial("InstancedGrass", kernelsConfiguration, screenSizeFBO, depthRenderer->getColorTexture());

}

void SampleEngine::useSplatting(bool useBestSample)
{
	grassPostProcess = NULL;
	grassSplatted = NULL;
	grassInstanced = NULL;
	grassPeeled = NULL;
	grassVolumic = NULL;

	delete kernelsConfiguration;
	kernelsConfiguration = new KernelList();
	kernelsConfiguration->loadFromFolder(ressourceObjPath + "Plane/Grass");
	kernelsConfiguration->setGridHeight(1.0);
	kernelsConfiguration->prepareGPUData();
	kernelsConfiguration->updateGPUData();
	kernelsConfiguration->bindBuffer(3);
	kernelsConfiguration->loadAnttweakMenu();

	//grassInstanced = new InstancedKernelMaterial("InstancedGrass", kernelsConfiguration, screenSizeFBO, mainRenderer->getDepthTexture());
	grassSplatted = new InstancedSplattedKernelMaterial("GrassSplatted", kernelsConfiguration, screenSizeFBO, depthRenderer->getColorTexture());
	grassSplatted->useBestFragment = useBestSample;

	//grassPeeled = new DepthPeeledShellMaterial("GrassPeeled", kernelsConfiguration, screenSizeFBO, mainRenderer->getDepthTexture());
	//grassPostProcess = new GrassComposer("GrassAdder", kernelsConfiguration, this);
	//grassPostProcess->setComposerFor(ground);
}


void SampleEngine::useComposer()
{
	grassPostProcess = NULL;
	grassSplatted = NULL;
	grassInstanced = NULL;
	grassPeeled = NULL;
	grassVolumic = NULL;

	delete kernelsConfiguration;
	kernelsConfiguration = new KernelList();
	kernelsConfiguration->loadFromFolder(ressourceObjPath + "Plane/Grass");
	kernelsConfiguration->setGridHeight(1.0);
	kernelsConfiguration->prepareGPUData();
	kernelsConfiguration->updateGPUData();
	kernelsConfiguration->bindBuffer(3);
	kernelsConfiguration->loadAnttweakMenu();

	//grassPeeled = new DepthPeeledShellMaterial("GrassPeeled", kernelsConfiguration, screenSizeFBO, mainRenderer->getDepthTexture());
	grassPostProcess = new GrassComposer("GrassAdder", kernelsConfiguration, this);
	grassPostProcess->setComposerFor(ground);
}

void SampleEngine::changeSplatting()
{
	grassSplatted->useSplatting = !(grassSplatted->useSplatting);
}

void SampleEngine::changeBestFragment()
{
	grassSplatted->useBestFragment = !(grassSplatted->useBestFragment);
}