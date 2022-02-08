/*
EngineGL overloaded for custom rendering
*/
#include "SampleEngine.h"
#include "Engine/OpenGL/v4/GLProgram.h"
#include "Engine/OpenGL/SceneLoaderGL.h"
#include "Engine/Base/NodeCollectors/MeshNodeCollector.h"
#include "Engine/Base/NodeCollectors/FCCollector.h"


#include <AntTweakBar/AntTweakBar.h>

#include "GPUResources/Textures/GPUTextureCubeMap.h"

#include "ParticleManager.h"
#include "Materials/ColorMaterial/ColorMaterial.h"

#include "GPUResources/GPUInfo.h"
#include "Effects/DepthCopy/DepthCopy.h"

float exposure = 1.0f;
float lastExposure = 1.0f;

std::vector<PBRMaterial*> pbrMaterials;
int idMaterial = 0, idLastMaterial = 1;

std::vector<LightNode*> lightNodes;
int idLightNode = 0;
glm::vec3 lightNodeColor = glm::vec3 (1);


bool usePostProcess = true;

// Pour gérer les matériaux
glm::vec3 baseColor;
float reflectance = 0.5f;
float metalMask = 0.0f;

glm::vec3 backgroundColor;

PostProcess *PP;

DepthCopy* depthFromFBO;

void addLight (std::string name, LightMaterial* lMat, glm::vec3 &position, int type, glm::vec4 &color);
void addObject (string name, string path, glm::vec3 &scale, glm::vec3 &translate, glm::vec4 &rotate, Material* m);

SampleEngine::SampleEngine (int width, int height) :
EngineGL (width, height)
{
	this->scene = Scene::getInstance ();
	this->allNodes = new StandardCollector ();
	this->renderedNodes = new MeshNodeCollector ();
	//this->renderedNodes = new FCCollector();
	w_Width = width;
	w_Height = height;
	this->lightingModel = NULL;
	timeQuery = new GPUQuery ("Timer", GL_TIME_ELAPSED);
	scene->needupdate = true;
	drawLights = false;
	drawBoundingBoxes = false;
	smoothness = 0.86;

	deferredMat = NULL;
	GeometryPassFBO = NULL;
	PostProcessFBO = NULL;
	FBO_Grain = NULL;
	grainEffect = NULL;

	texDamier = NULL;
	texDamierGold = NULL;
	normalDamier = NULL;
	roughnessDamier = NULL;
	texWood = NULL;
	normalWood = NULL;

}

SampleEngine::~SampleEngine ()
{
}

// Appelé par AntTweakBar pour l'activation du PostProcess
void TW_CALL RunCB (void * /*clientData*/) {
	usePostProcess = !usePostProcess;

}

void TW_CALL SwitchDSSDO (void * /*clientData*/) {
	PP->useDO = !PP->useDO;
}

bool SampleEngine::init ()
{
	LOG (INFO) << "Initializing Scene";
	timeQuery->create ();

	
	std::cout << GPUInfo::Instance()->getOpenGLVersion() << std::endl;

	std::cout << "Max Color Attachments : " << GPUInfo::Instance()->getMaximumColorAttachments() << std::endl;
	std::cout << "Max Drawing Buffers : " << GPUInfo::Instance()->getMaximumDrawBuffers() << std::endl;

	// Load shaders in "\common" into graphic cards
	GLProgram::prgMgr.addPath (ressourceMaterialPath + "Common", "");

	texDamier = scene->getResource<GPUTexture2D> (ressourceTexPath + "damier.png");
	texDamierGold = scene->getResource<GPUTexture2D> (ressourceTexPath + "damierHDGold.png");
	normalDamier = scene->getResource<GPUTexture2D> (ressourceTexPath + "damierHD_n.png");
	roughnessDamier = scene->getResource<GPUTexture2D> (ressourceTexPath + "damierHDRoughness.png");
	texWood = scene->getResource<GPUTexture2D> (ressourceTexPath + "Cedar_shake_pxr128.png");
	normalWood = scene->getResource<GPUTexture2D> (ressourceTexPath + "Cedar_shake_pxr128_normal.png");
	
	//dirt = scene->getResource<GPUTexture2D>(ressourceObjPath + "color.png");
	//dirt = scene->getResource<GPUTexture2D>(ressourceObjPath + "Falaise2/Guillaume_remesh_Color.png");
	dirt = scene->getResource<GPUTexture2D>(ressourceObjPath + "Dragon/dirt.jpg");


	pbrMaterials.push_back (new PBRMaterial ("PBR" + std::to_string (pbrMaterials.size ())));
	pbrMaterials.push_back (new PBRMaterial ("PBR" + std::to_string (pbrMaterials.size ())));
	pbrMaterials.push_back (new PBRMaterial ("PBR" + std::to_string (pbrMaterials.size ())));
	pbrMaterials.push_back (new PBRMaterial ("PBR" + std::to_string (pbrMaterials.size ())));
	pbrMaterials.push_back (new PBRMaterial ("PBR" + std::to_string (pbrMaterials.size ())));

	defaultMaterial = new ColorMaterial("test", glm::vec4(0.5, 0.5, 0.5, 1.0));
	groundDefault = new TextureMaterial("Ground", dirt);

	PostProcessFBO = new GPUFBO ("Post Process Passes");
	PostProcessFBO->create (FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
	
	FBO_Grain = new GPUFBO ("Grain FBO");
	FBO_Grain->create (FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
	grainEffect = new GrainEffect ("Grain Effect");

	bool useAA = false;
	bool useAO = true;
	float numSamples = 64;
	int noiseSize = 16;
	bool useTM = true;
	bool useTemporalTM = true;
	bool useBl = false;
	float threshold = 0.9f;
	int blurSize = 4;
	bool useDOF = false;
	bool useNearDOF = false;
	bool useFarDOF = false;
	float nearFocal = 1.0f;
	float farFocal = 64;
	int focalBlurSize = 8;
	int shadingBlurSize = 8;
	bool useMB = false;
	bool useFog = false;
	bool useFinal = false;
	PP = new PostProcess ("Post Process",
		true,
		useAA, useAO, numSamples, noiseSize, useTM, useTemporalTM, exposure, useBl, threshold, blurSize,
		useDOF, useNearDOF, useFarDOF, nearFocal, farFocal, focalBlurSize, shadingBlurSize,
		useFog, useMB, useFinal);


	GeometryPassFBO = new GPUFBO ("Geometry Pass");
	GeometryPassFBO->create (FBO_WIDTH, FBO_HEIGHT, 1, true, GL_RGB32F, GL_TEXTURE_2D_ARRAY, 6);

	screenSizeFBO = new GPUFBO("ForAccumulationInMaterial");
	screenSizeFBO->create(FBO_WIDTH, FBO_HEIGHT, 3, true, GL_RGBA32F, GL_TEXTURE_2D, 1);

	mainFrameBuffer = new GPUFBO("mainFrameBuffer");
	mainFrameBuffer->create(FBO_WIDTH, FBO_HEIGHT, 1, true, GL_RGBA32F, GL_TEXTURE_2D, 1);

	depthFromFBO = new DepthCopy("ZBufferCopy");

	

	deferredMat = new DeferredMaterial ("DeferredMaterialTest",
		Scene::getInstance ()->getResource<GPUTexture2D> (ressourceTexPath + "wrwoodplanks01.png"),
		NULL,
		NULL,
		NULL,
		NULL,
		Scene::getInstance ()->getResource<GPUTexture2D> (ressourceTexPath + "wrwoodplanks01_n.png"),
		0.05f, 32, false, false,
		glm::vec3 (0.0, 0.4, 0.68),
		glm::vec3 (0.5));


	bendGrass = new BendGrass("BendGrass");



	/////////////////////////////
	/////Kernel profile//////////
	/////////////////////////////
	kernelsConfiguration = new KernelList();
	//kernelsConfiguration->loadFromFolder(ressourceObjPath + "Plane/GrassInstancing");
	//kernelsConfiguration->loadFromFolder(ressourceObjPath + "Plane/Grass_profile");
	kernelsConfiguration->loadFromFolder(ressourceObjPath + "FurryBunny/FurElements_profile");
	//kernelsConfiguration->loadFromFolder(ressourceObjPath + "Volumes/Testing");
	//kernelsConfiguration->loadFromFolder(ressourceObjPath + "Volumes/Config_lichen");
	//kernelsConfiguration->loadFromFolder(ressourceObjPath + "Falaise2/Kernels");
	//kernelsConfiguration->loadFromFolder(ressourceObjPath + "Dragon/Kernels");
	//kernelsConfiguration->loadFromFolder(ressourceObjPath + "Plane_old/Grass");

	kernelsConfiguration->prepareGPUData();
	kernelsConfiguration->updateGPUData();
	kernelsConfiguration->bindBuffer(3);
	kernelsConfiguration->loadAnttweakMenu();

	flatKernelsRendering = NULL;
	implicitSurfaceKernel = NULL;
	kernel3DTest = NULL;
	implicitSurfaceKernel = NULL;
	
	//grassInstanced = new InstancedKernelMaterial("InstancedGrass", kernelsConfiguration, screenSizeFBO, mainRenderer->getDepthTexture());
	flatKernelsRendering = new FlatKernelsOnMesh("FlatKernels", kernelsConfiguration, screenSizeFBO, mainFrameBuffer->getDepthTexture());
	//volumetricKernelsRendering = new VolumetricKernelsOnMesh("VolumetricKernels", kernelsConfiguration, screenSizeFBO, mainFrameBuffer->getDepthTexture());
	//implicitSurfaceKernel = new ImplicitSurfaceMaterial("Surfaces", kernelsConfiguration, screenSizeFBO, NULL);
	//kernel3DTest = new Deferred3DKernelMaterial("HEYYY", kernelsConfiguration, glm::vec3(0.0, 0.4, 0.68));

	

	LightMaterial* lmat = new LightMaterial ("LightMaterial-" + string ("DefaultLight"), glm::vec4 (1));
	lightNodeColor = glm::vec3 (1);
	//addLight ("MyLight", lmat, glm::vec3 (0.0, 0.0, 30.0), SPOT_LIGHT, glm::vec4 (1, 0, 1, 1));
	addLight( "SunLight", lmat, glm::vec3( -160, 120.0, 160 ), SPOT_LIGHT, glm::vec4( 1, 1, 1, 1 ) );
	//addLight ("MyLight", lmat, glm::vec3 (0, 30.0, 50), SPOT_LIGHT, glm::vec4 (1, 0, 0, 1));
	//addLight ("MyLight2", lmat, glm::vec3 (0, 50.0, -120), SPOT_LIGHT, glm::vec4 (glm::vec3 (0.6), 1));
	//addLight ("RedLight", lmat, glm::vec3 (180, 70.0, 0), SPOT_LIGHT, glm::vec4 (0.6, 0, 0, 1));
	//addLight ("BlueLight", lmat, glm::vec3 (-180, 70.0, 0), SPOT_LIGHT, glm::vec4 (0, 0, 0.6, 1));
	//addLight ("BlueLight", lmat, glm::vec3 (60, 20.0, 0), SPOT_LIGHT, glm::vec4 (1, 1, 1, 1));


	pbrMaterials.push_back (new PBRMaterial ("PBRMat1", glm::vec3 (0.5), 0.25, 1.0, 0.0));
	pbrMaterials.push_back (new PBRMaterial ("PBRMat2", glm::vec3 (0.5), 0.5, 1.0, 0.0));
	pbrMaterials.push_back (new PBRMaterial ("PBRMat3", glm::vec3 (0.5), 0.75, 1.0, 0.0));
	pbrMaterials.push_back (new PBRMaterial ("PBRMat4", glm::vec3 (0.5), 0.95, 1.0, 0.0));
	pbrMaterials.push_back (new PBRMaterial ("PBRMat5", glm::vec3 (0.5), 0.95, 1.0, 0.0));
	pbrMaterials.push_back (new PBRMaterial ("PBRMat6", glm::vec3 (0.5), 0.95, 1.0, 0.0));

	float disp = 12;
	float scale = 0.5;

	//addObject ("Arc7", "teapot4.obj", glm::vec3 (0.05), glm::vec3 (10, 0, 0), glm::vec4 (0, 1, 0, 3.1415926535 / 2.0), pbrMaterials[1]);
	//addObject ("Arc8", "teapot4.obj", glm::vec3 (0.05), glm::vec3 (-10, 0, 0), glm::vec4 (0, -1, 0, 3.1415926535 / 2.0), pbrMaterials[2]);
	//addObject ("Mitsuba", "mitsubaTest.obj", glm::vec3 (1), glm::vec3 (0, 0, 0), glm::vec4 (0, 1, 0, 3.1415926535f), pbrMaterials[2]);
	/*Node* sceneLoaded2 = scene->getNode ("Mitsuba");
	ModelGL *planeModel = new ModelGL ("Johnny", false);
	planeModel = scene->getModel<ModelGL> (ressourceObjPath + "mitsubaTest.obj");
	sceneLoaded2->frame ()->scale (glm::vec3 (5.3));
	sceneLoaded2->setAnimator (new RotationAnimator (sceneLoaded2->frame (), 0.987654321, glm::vec3 (0, 1, 0)));
	sceneLoaded2->frame ()->translate (glm::vec3 (0, 0, 0));
	sceneLoaded2->setModel (planeModel);
	sceneLoaded2->setMaterial (pbrMaterials[0], true);
	scene->getRoot ()->adopt (sceneLoaded2);*/

	 
	float size = 220;
	//addObject ("PlaneBottom", "Quad.obj", glm::vec3 (size), glm::vec3 (0, 0, 0), glm::vec4 (1, 0, 0, 3.14 / 2.0), pbrMaterials[4]);
	/*addObject ("PlaneFar", "Quad.obj", glm::vec3 (size), glm::vec3 (0, size, -size), glm::vec4 (1, 0, 0, 3.1415926535), pbrMaterials[1]);
	addObject ("PlaneLeft", "Quad.obj", glm::vec3 (size), glm::vec3 (size, size, 0), glm::vec4 (0, 1, 0, 3.1415926535 / 2.0), pbrMaterials[2]);
	addObject ("PlaneRight", "Quad.obj", glm::vec3 (size), glm::vec3 (-size, size, 0), glm::vec4 (0, -1, 0, 3.1415926535 / 2.0), pbrMaterials[3]);
	addObject ("PlaneNear", "Quad.obj", glm::vec3 (size), glm::vec3 (0, size, size), glm::vec4 (0), pbrMaterials[4]);
	*/

	

	// Create Lighting Model GL and collect all light nodes of the scene 
	lightingModel = new LightingModelGL ("LightingModel", scene->getRoot ());

	//chargement de sponza
	//addObject ("Sponza", "sponza.obj", glm::vec3 (0.082), glm::vec3 (0, 0, 0), glm::vec4 (0, 1, 0, 3.1415926535f*0.5f), pbrMaterials[3]);
	//Node* sceneLoaded = sceneloader->loadScene( ressourceObjPath + "sponza.obj" );
	//sceneLoaded->frame()->scale( glm::vec3( 0.082 ) );
	//sceneLoaded->frame()->rotate( glm::vec3( 0, 1, 0 ), 3.1415926535f*0.5f );
	//addObject("Bunny", "FurryBunny/Bunny.obj", glm::vec3(1.0), glm::vec3(0, 10.0, 0.0), glm::vec4(0, 1, 0, -3.1415926535f), deferredMat);
	SceneLoaderGL* sceneloader = new SceneLoaderGL();
	
	//Node* sceneLoaded = sceneloader->loadScene(ressourceObjPath + "Plane/Plane.DAE");
	Node* sceneLoaded = sceneloader->loadScene(ressourceObjPath + "FurryBunny/Bunny.obj");
	//Node* sceneLoaded = sceneloader->loadScene(ressourceObjPath + "Falaise2/Falaise2.DAE");
	//Node* sceneLoaded = sceneloader->loadScene(ressourceObjPath + "Dragon/Dragon.DAE");
	scene->getSceneNode()->adopt( sceneLoaded ); 

	// Base node
	ground = scene->getNode("Bunny");
	//ground = scene->getNode("Sol");
	//ground = scene->getNode("Dragon");
	
	// Base material
	ground->setMaterial(groundDefault);
	//ground->setMaterial(defaultMaterial);

	// Copy to render the shell
	shellNode = new Node(*ground);
	//shellNode->setMaterial(volumetricKernelsRendering);
	shellNode->setMaterial(flatKernelsRendering);


	//ground->frame()->scale(glm::vec3(0.082));
	//ground->frame()->rotate(glm::vec3(0, 1, 0), 3.1415926535f*0.5f);

	//pierre = Scene::getInstance()->getNode("Pierre");
	//pierre->setModel(Scene::getInstance()->getModel<ModelGL>(ressourceObjPath + "Sphere.obj"));
	//pierre->setMaterial(new ColorMaterial("PiereMat", glm::vec4(0.7, 0.0, 0.2, 1.0)));
	//pierre->frame()->scale(glm::vec3(50.0));
	//scene->getSceneNode()->adopt(pierre);


	// Create Bounding Box Material for bounding box rendering
	boundingBoxMat = new BoundingBoxMaterial ("BoundingBoxMat");

	// OpenGL state variable initialisation
	glClearColor (0.0, 0.0, 0.0, 1.0);
	glEnable (GL_DEPTH_TEST);


	// Force window Resize
	this->onWindowResize (w_Width, w_Height);

	// Bind the light buffer for rendering
	lightingModel->bind ();

	allNodes->collect (scene->getRoot ());

	renderedNodes->collect (scene->getRoot ());

	for (unsigned int i = 0; i < allNodes->nodes.size (); i++)
		allNodes->nodes[i]->animate (0);



	// ---------------- Interface

	baseColor = glm::vec3 (0.5);
	metalMask = 0.0f;
	reflectance = 0.5f;
	F0 = glm::vec3 (0.04);
	albedo = glm::vec3 (0.5);
	//lightPos = glm::vec3 (0, 0, -12);
	backgroundColor = glm::vec3 (0.7f, 0.93f, 1.0f);


	
	TwBar *myBar;
	myBar = TwNewBar ("Control Bar");

	TwAddVarRW (myBar, "Smoothness", TW_TYPE_FLOAT, &smoothness, " min=0.000001 max=1 step=0.001 label='Smoothness' ");
	TwAddVarRW (myBar, "Reflectance", TW_TYPE_FLOAT, &reflectance, " min=0.0000000000001 max=1 step=0.01 label='Reflectance' ");
	TwAddVarRW (myBar, "Metal", TW_TYPE_FLOAT, &metalMask, " min=0 max=1 step=0.2 label='Metal' ");
	TwAddVarRW (myBar, "Albedo Red", TW_TYPE_FLOAT, &baseColor.r, " min=0 max=1 step=0.002 group = 'Albedo' label='Red' ");
	TwAddVarRW (myBar, "Albedo Green", TW_TYPE_FLOAT, &baseColor.g, " min=0 max=1 step=0.002 group = 'Albedo' label='Green' ");
	TwAddVarRW (myBar, "Albedo Blue", TW_TYPE_FLOAT, &baseColor.b, " min=0 max=1 step=0.002 group = 'Albedo' label='Blue' ");

	TwAddVarRW (myBar, "BackGround Red", TW_TYPE_FLOAT, &backgroundColor.r, " min=0 max=1 step=0.01 group = 'BackGround' label='Red' ");
	TwAddVarRW (myBar, "BackGround Green", TW_TYPE_FLOAT, &backgroundColor.g, " min=0 max=1 step=0.01 group = 'BackGround' label='Green' ");
	TwAddVarRW (myBar, "BackGround Blue", TW_TYPE_FLOAT, &backgroundColor.b, " min=0 max=1 step=0.01 group = 'BackGround' label='Blue' ");

	string s = " min=0 max=" + std::to_string (pbrMaterials.size () - 1) + " step=1 label='Material Id'";
	const char* c = s.c_str ();
	TwAddVarRW (myBar, "Material Id", TW_TYPE_INT8, &idMaterial, c);

	TwAddButton (myBar, "PostProcess", RunCB, NULL, " label='PostProcess' ");
	TwAddButton (myBar, "DSSDO", SwitchDSSDO, NULL, " group='DSSDO' label='_DSSDO' ");
	TwAddVarRW (myBar, "DSSDO Base Radius", TW_TYPE_FLOAT, &PP->doBaseRadiusVal, " min=0.0001 max=2 step=0.0001 group='DSSDO' label='Radius' ");
	TwAddVarRW (myBar, "DSSDO Max Occlusion Dist", TW_TYPE_FLOAT, &PP->doMaxOcclusionDistVal, " min=0.001 max=40.0 step=0.01 group='DSSDO' label='Max' ");

//	TwAddVarRW (myBar, "Rotation Speed", TW_TYPE_FLOAT, &static_cast<RotationAnimator*>(sceneLoaded2->animator())->rotationSpeed, " min=-2 max=2 step=0.001 label='Rotatio Speed' ");

	s = " min=0 max=" + std::to_string (lightNodes.size () - 1) + " step=1 label='LightNode Id'";
	c = s.c_str ();
	TwAddVarRW (myBar, "LightNode Id", TW_TYPE_INT8, &idLightNode, c);
	TwAddVarRW (myBar, "LightNode Red", TW_TYPE_FLOAT, &lightNodeColor.r, " min=0 max=100000 step=0.1 group = 'LightNode' label='Red' ");
	TwAddVarRW (myBar, "LightNode Green", TW_TYPE_FLOAT, &lightNodeColor.g, " min=0 max=100000 step=0.1 group = 'LightNode' label='Green' ");
	TwAddVarRW (myBar, "LightNode Blue", TW_TYPE_FLOAT, &lightNodeColor.b, " min=0 max=100000 step=0.1 group = 'LightNode' label='Blue' ");

	TwAddVarRW (myBar, "Exposure", TW_TYPE_FLOAT, &exposure, " min=0.00000001 max=15 step=0.001 label='Exposure' ");
	// -----------------

	std::cout << scene->camera()->getZnear() << " ; " << scene->camera()->getZfar() << std::endl;

	kernelsConfiguration->setGridHeight(0.0);

	return(true);
}


void SampleEngine::render ()
{
	
	// Begin Time query
	timeQuery->begin ();

	mainFrameBuffer->enable();
	mainFrameBuffer->drawBuffers(1);

	kernelsConfiguration->updateGPUData();

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//Rendering every collected node
	for (unsigned int i = 0; i < renderedNodes->nodes.size (); i++)
		renderedNodes->nodes[i]->render();
	
	// For instancing only
	//if (flatKernelsRendering->useInstacingOnly)	ground->render(flatKernelsRendering);
	//if (volumetricKernelsRendering->useInstacingOnly)	ground->render(volumetricKernelsRendering);

	if (drawLights)
		lightingModel->renderLights();

	if (drawBoundingBoxes)
		for (unsigned int i = 0; i < renderedNodes->nodes.size(); i++)
			renderedNodes->nodes[i]->render(boundingBoxMat);

	//ground->render(grassSplatted);
	//ground->render(implicitSurfaceKernel);
	//ground->render(testVolumeSplatting);


		// -- Deferred rendering pass
		//GeometryPassFBO->enable();
		//glDepthMask (GL_TRUE);
		//glClearColor (0.0, 0.0, 0.0, 1.0);
		//glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glEnable (GL_DEPTH_TEST);
		//
		//GeometryPassFBO->drawBuffers(6);
		//
		////Rendering every collected node
		//for (unsigned int i = 0; i < renderedNodes->nodes.size (); i++)
		//	renderedNodes->nodes[i]->render();
		//
		//// For iso surface deferred rendering
		//ground->render(kernel3DTest);
		//
		//glDisable (GL_DEPTH_TEST);
		//glDepthMask (GL_FALSE);
		//GeometryPassFBO->disable();
		//
		//// Display
		//PP->apply(GeometryPassFBO, PostProcessFBO); // Application du PostProcess
		//PostProcessFBO->display (); // Affichage du FBO contenant l'image finale
		
		
		
		
		// Depth copy from geometry pass
		//glDepthMask(GL_TRUE);
		//glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		//depthFromFBO->display((GPUTexture2D*) GeometryPassFBO->getDepthTexture());
		//glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		//
		////ground->render(grassSplatted);
		//ground->render(implicitSurfaceKernel);
		//ground->render(testVolumeSplatting);


		
		//grainEffect->apply (PostProcessFBO, FBO_Grain); // Problèmes ToneMapping
		//FBO_Grain->display ();
	//}
	
		//depthRenderer->display(glm::vec4(0, 0, 0.25, 0.25));

	mainFrameBuffer->disable();
	mainFrameBuffer->display();

	shellNode->render();
	//if (!flatKernelsRendering->useInstacingOnly)	ground->render(flatKernelsRendering);
	//if (!volumetricKernelsRendering->useInstacingOnly)	ground->render(volumetricKernelsRendering);
	
	// end time Query					
	timeQuery->end ();

	// scene has been rendered : no need for recomputing values until next camera/parameter change
	//Scene::getInstance()->camera()->setUpdate(true);
	scene->needupdate = false;

}
float teta = 0.0f;
void SampleEngine::animate (const int elapsedTime)
{
	// ---- Interface AntTweakBar
	if (!usePostProcess) {
		if (idMaterial != idLastMaterial) {
			idLastMaterial = idMaterial;
			smoothness = pbrMaterials[idMaterial]->getSmoothness ();
			reflectance = pbrMaterials[idMaterial]->getReflectance ();
			baseColor = pbrMaterials[idMaterial]->getAlbedo ();
			metalMask = pbrMaterials[idMaterial]->getMetalMask ();
		}
		else {
			pbrMaterials[idMaterial]->updateValues (smoothness, metalMask, reflectance, baseColor);
		}
	}
	else {
		if (lastExposure != exposure) {
			lastExposure = exposure;
			PP->setExposure (exposure);
		}
	}
	lightNodes[idLightNode]->setColor (glm::vec4 (lightNodeColor, 1.0f));
	// ---- 


	teta += elapsedTime*0.00007987654321;

	// Animate each node
	for (unsigned int i = 0; i < allNodes->nodes.size (); i++)
		allNodes->nodes[i]->animate (elapsedTime);


	// force update of lighting model
	//Scene::getInstance()->camera()->updateBuffer();
	lightingModel->update (true);

}

void SampleEngine::onWindowResize (int width, int height)
{

	glViewport (0, 0, width, height);
	scene->camera ()->setAspectRatio ((float)width / (float)height);
	w_Width = width;
	w_Height = height;

	if (lightingModel != NULL)
		lightingModel->setWindowSize (glm::vec2 (width, height));

}


void addLight (string name, LightMaterial* lMat, glm::vec3 &position, int type, glm::vec4 &color) {


	LOG (INFO) << "Adding Light : " << name;
	LightNode* lnode = new LightNode (name);
	Scene::getInstance ()->m_Nodes.insert (lnode->getName (), lnode);		// insert light node into node manager

	lnode->setModel (Scene::getInstance ()->getModel<ModelGL> (ressourceObjPath + "Sphere.obj"));
	lnode->setMaterial (lMat);

	Scene::getInstance ()->getRoot ()->adopt (lnode);

	lnode->frame ()->translate (position);
	lnode->frame ()->scale (glm::vec3 (20.0, 20.0, 20.0));
	lnode->setPosition (glm::vec4 (position, 0.0));
	lnode->setType (type);


	lnode->setColor (color);

	lightNodes.push_back (lnode);
}

void addObject (string name, string path, glm::vec3 &scale, glm::vec3 &translate, glm::vec4 &rotate, Material* m) {
	Node* sceneLoaded = Scene::getInstance ()->getNode (name);
	ModelGL *planeModel = new ModelGL (name + "Model", false);
	planeModel = Scene::getInstance ()->getModel<ModelGL> (ressourceObjPath + path);

	sceneLoaded->frame ()->translate (translate);
	sceneLoaded->frame ()->scale (scale);

	if (rotate.x != 0 || rotate.y != 0 || rotate.z != 0 && rotate.w != 0)
		sceneLoaded->frame ()->rotate (glm::vec3 (rotate), rotate.w);

	sceneLoaded->setModel (planeModel);
	sceneLoaded->setMaterial (m, true);
	Scene::getInstance ()->getRoot ()->adopt (sceneLoaded);
}

void SampleEngine::reloadPostProcess () {
	glClearColor (1.0, 0.0, 0.0, 1.0);
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	GeometryPassFBO->~GPUFBO ();
	GeometryPassFBO = new GPUFBO ("Geometry Pass");
	GeometryPassFBO->create (FBO_WIDTH, FBO_HEIGHT, 1, true, GL_RGB16F, GL_TEXTURE_2D_ARRAY, 7);

	PostProcessFBO->~GPUFBO ();
	PostProcessFBO = new GPUFBO ("Post Process Passes");
	PostProcessFBO->create (FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);

	bool useAA = true;
	bool useAO = true;
	float numSamples = 64;
	int noiseSize = 16;
	bool useTM = true;
	bool useTemporalTM = true;
	bool useBl = false;
	float threshold = 0.9f;
	int blurSize = 4;
	bool useDOF = false;
	bool useNearDOF = false;
	bool useFarDOF = false;
	float nearFocal = 1.0f;
	float farFocal = 64;
	int focalBlurSize = 8;
	int shadingBlurSize = 8;
	useMB = !useMB;
	bool useFog = false;
	bool useFinal = false;
	PP = new PostProcess ("Post Process",
		true,
		useAA, useAO, numSamples, noiseSize, useTM, useTemporalTM, exposure, useBl, threshold, blurSize,
		useDOF, useNearDOF, useFarDOF, nearFocal, farFocal, focalBlurSize, shadingBlurSize,
		useFog, useMB, useFinal);

	cout << "Motion Blur :" << useMB << endl;
}