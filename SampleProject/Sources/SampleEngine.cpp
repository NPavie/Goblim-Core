/*
	EngineGL overloaded for custom rendering
*/
#include "SampleEngine.h"
#include "Engine/OpenGL/v4/GLProgram.h"
#include "Engine/OpenGL/SceneLoaderGL.h"
#include "Engine/Base/NodeCollectors/MeshNodeCollector.h"
#include "Engine/Base/NodeCollectors/FCCollector.h"

#include "GPUResources/GPUInfo.h"

#include "GPUSpotVector.h"
#include "Effects/SpotNoise/SpotNoise.h"
#include "Effects/DistributionProfile/DistributionProfile.h"
#include "Effects/SpotProfile/SpotProfile.h"
#include "Materials/SurfacicSpotNoise/SurfacicSpotNoise.h"

#include "GaussianSplat.h"

#include <Utils/dirent.h>
#ifdef _MSC_VER
	
#endif
int nbLvlToDisplay = 1;

SpotNoise *noise;
SpotProfile *spotProfile, *distribSpotProfile;
SpotProfile *spotProfile2;
DistributionProfile *distribution;
GPUSpotVector *spotLoader;

SurfacicSpotNoise* noiseOnMesh;



SampleEngine::SampleEngine(int width, int height):
EngineGL(width,height)
{

}
SampleEngine::~SampleEngine()
{
	
}

typedef struct justForFun {
	int val1;
	float val2;
} justForFun;

bool SampleEngine::init()
{
	activeRecord = false;

	justForFun * test = (justForFun*)malloc(3 * sizeof(justForFun));
	for (int i = 0; i < 3; ++i) {
		test[i].val1 = i;
		test[i].val2 = i;
	}

	// --------------
	// Exemple d'un Engine chargeant un lapin initialement situé dans SampleProject/Objets/FurryBunny
	// --------------
	LOG(INFO) << "Initializing Scene";
	timeQuery->create();

	// Load shaders in "\common" into graphic cards
	GLProgram::prgMgr.addPath(ressourceMaterialPath + "Common", "");

	// Loading a scene or a mesh and add it to the root of the Engine scenegraphe
	SceneLoaderGL* sceneloader = new SceneLoaderGL();
	//Node* sceneLoaded = sceneloader->loadScene(ressourceObjPath + "Dragon.DAE");
	Node* sceneLoaded = sceneloader->loadScene(ressourceObjPath + "FurryBunny/Bunny.obj");
	//Node* sceneLoaded = sceneloader->loadScene(ressourceObjPath + "Plane/Plane.DAE");
	scene->getSceneNode()->adopt(sceneLoaded);

	// Create Lighting Model GL and collect all light nodes of the scene 
	lightingModel = new LightingModelGL("LightingModel", scene->getRoot());

	// Create Bounding Box Material for bounding box rendering
	boundingBoxMat = new BoundingBoxMaterial("BoundingBoxMat");

	Spot* constantSpot = new Spot();
	constantSpot->addFunction(ADD, new Constant(0.5));

	Spot* gaussianSpot = new Spot();
	gaussianSpot->addFunction(ADD, 
		new Gaussian(1.0, glm::vec3(0.0), glm::vec3(0.0), glm::vec3(4.0f / 4.29f, 4.0f / 4.29f,4.0f / 4.29f))
		);
	//gaussianSpot->getGaussiansVector()->at(0).scale.w = gaussianSpot->getGaussiansVector()->at(0).getDeterminant() / 6.28f;
	
	Spot* gaussianSpot2 = new Spot();
	gaussianSpot2->addFunction(ADD, 
		new Gaussian(0.5, glm::vec3(0.0), glm::vec3(0.0), glm::vec3(1.0f / 4.29f, 1.0f / 4.29f, 1.0f / 4.29f))
		);
	//gaussianSpot2->getGaussiansVector()->at(0).scale.w = gaussianSpot->getGaussiansVector()->at(0).getDeterminant() / 6.28f;
	
	Spot* circleSpot = new Spot();
	circleSpot->addFunction(ADD,
		new Gaussian(100.0, glm::vec3(0.0), glm::vec3(0.0), glm::vec3(0.5f / 4.29f, 0.5f / 4.29f, 1.0f))
		);

	// oriented gabor spot
	Spot* gaborSpot = new Spot();
	gaborSpot->addFunction(ADD, new Harmonic(4.0f, glm::vec2(0.785, 0.0)));
	gaborSpot->addFunction(MULT, new Gaussian(2.0, glm::vec3(0.0), glm::vec3(0.0), glm::vec3(1.0f / 4.29f, 1.0f / 4.29f, 1.0f)));
	
	
	//Spot* crossSpot = new Spot();
	//crossSpot->addFunction(ADD, new Gaussian(1.0, glm::vec3(0), glm::vec3(0.0), glm::vec3(0.5f / 4.29f, 0.05f / 4.29f, 0.5f / 4.29f)));
	//crossSpot->getGaussiansVector()->at(0).scale.w = crossSpot->getGaussiansVector()->at(0).getDeterminant() / 6.28f;
	//crossSpot->addFunction(ADD, new Gaussian(1.0, glm::vec3(0), glm::vec3(0.0), glm::vec3(0.05f / 4.29f, 0.5f / 4.29f, 0.5f / 4.29f)));
	//crossSpot->getGaussiansVector()->at(1).scale.w = crossSpot->getGaussiansVector()->at(1).getDeterminant() / 6.28f;

	Spot* crossSpot = new Spot();
	crossSpot->addFunction(ADD, new Gaussian(2.0, glm::vec3(0), glm::vec3(0.0), glm::vec3(1.0f / 4.29f, 0.1f / 4.29f, 1.0f)));
	//crossSpot->getGaussiansVector()->at(0).scale.w = 0.1 * crossSpot->getGaussiansVector()->at(0).getDeterminant() / 6.28f;
	crossSpot->addFunction(ADD, new Gaussian(2.0, glm::vec3(0), glm::vec3(0.0), glm::vec3(0.1f / 4.29f, 1.0f / 4.29f, 1.0f)));
	//crossSpot->getGaussiansVector()->at(1).scale.w =  0.25 * crossSpot->getGaussiansVector()->at(1).getDeterminant() / 6.28f;

	Spot* starSpot = new Spot();
	starSpot->addFunction(ADD, new Gaussian(2.0, glm::vec3(0), glm::vec3(0.0), glm::vec3(1.0f / 4.29f, 0.1f / 4.29f, 1.0f)));
	//crossSpot->getGaussiansVector()->at(0).scale.w = 0.1 * crossSpot->getGaussiansVector()->at(0).getDeterminant() / 6.28f;
	starSpot->addFunction(ADD, new Gaussian(2.0, glm::vec3(0), glm::vec3(0.0), glm::vec3(0.1f / 4.29f, 1.0f / 4.29f, 1.0f)));
	//crossSpot->getGaussiansVector()->at(1).scale.w =  0.25 * crossSpot->getGaussiansVector()->at(1).getDeterminant() / 6.28f;
	starSpot->addFunction(ADD, new Gaussian(2.0, glm::vec3(0), glm::vec3(0.0,0.0,3.14/4.0), glm::vec3(1.0f / 4.29f, 0.1f / 4.29f, 1.0f)));
	//crossSpot->getGaussiansVector()->at(0).scale.w = 0.1 * crossSpot->getGaussiansVector()->at(0).getDeterminant() / 6.28f;
	starSpot->addFunction(ADD, new Gaussian(2.0, glm::vec3(0), glm::vec3(0.0, 0.0,-3.14 / 4.0), glm::vec3(1.0f / 4.29f, 0.1f / 4.29f, 1.0f)));
	//crossSpot->getGaussiansVector()->at(1).scale.w =  0.25 * crossSpot->getGaussiansVector()->at(1).getDeterminant() / 6.28f;

	Spot* TSpot = new Spot();
	//TSpot->addFunction(ADD, new Gaussian(2.0, glm::vec3(0.25,0.25,0), glm::vec3(0.0,0.0,-3.14/4.0), glm::vec3(1.0f / 4.29f, 0.1f / 4.29f, 1.0f)));
	//TSpot->addFunction(ADD, new Gaussian(2.0, glm::vec3(0, 0.0, 0), glm::vec3(0.0,0.0, -3.14 / 4.0), glm::vec3(0.1f / 4.29f, 1.0f / 4.29f, 1.0f)));
	TSpot->addFunction(ADD, new Gaussian(2.0, glm::vec3(0.25, 0.25, 0), glm::vec3(0.0, 0.0, -3.14 / 4.0), glm::vec3(1.0f / 4.29f, 0.1f / 4.29f, 1.0f)));
	TSpot->addFunction(ADD, new Gaussian(2.0, glm::vec3(0, 0.0, 0), glm::vec3(0.0, 0.0, -3.14 / 4.0), glm::vec3(0.05, 0.4, 1.0f)));

	
	Spot* Hspot = new Spot();
	Hspot->addFunction(ADD, new Gaussian(2.0, glm::vec3(-0.25,0, 0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.1f / 4.29f, 1.0f / 4.29f, 1.0f)));
	Hspot->addFunction(ADD, new Gaussian(2.0, glm::vec3(0, 0, 0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0f / 4.29f, 0.1f / 4.29f, 1.0f)));
	Hspot->addFunction(ADD, new Gaussian(2.0, glm::vec3(0.25, 0, 0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.1f / 4.29f, 1.0f / 4.29f, 1.0f)));


	Spot* TriangleSpot = new Spot();
	TriangleSpot->addFunction(ADD, new Gaussian(2.0, glm::vec3(0.15, 0.0, 0), glm::vec3(0.0, 0.0, -3.14 / 4.0), glm::vec3(1.0f / 4.29f, 0.1f / 4.29f, 1.0f)));
	TriangleSpot->addFunction(ADD, new Gaussian(2.0, glm::vec3(-0.15, 0.0, 0), glm::vec3(0.0, 0.0, 3.14 / 4.0), glm::vec3(1.0f / 4.29f, 0.1f / 4.29f, 1.0f)));
	TriangleSpot->addFunction(ADD, new Gaussian(2.0, glm::vec3(0.0, -0.15, 0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(1.0f / 4.29f, 0.1f / 4.29f, 1.0f)));
	//TriangleSpot->addFunction(ADD, new Gaussian(2.0, glm::vec3(0.0, 0.0, 0), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.25f / 4.29f, 0.25f / 4.29f, 1.0f)));

	// SPOT_Scales
	Spot* Scales = new Spot(0.2);
	Scales->addFunction(ADD, 
		new Gaussian(0.15, 
			glm::vec3(0.30, -0.15, 0), 
			glm::vec3(0.0, 0.0, -0.785), 
			glm::vec3(0.25, 0.120, 1.0f),
			glm::vec3(1.0)));
	Scales->addFunction(ADD,
		new Gaussian(0.15,
			glm::vec3(0.0, -0.15, 0),
			glm::vec3(0.0, 0.0, 0.785),
			glm::vec3(0.25, 0.120, 1.0f),
			glm::vec3(1.0)));

	// SPOT_Cross
	Spot* cross = new Spot(0.175);
	cross->addFunction(ADD,
		new Gaussian(0.365,
			glm::vec3(0.0, 0.0, 0),
			glm::vec3(0.0, 0.0, -1.625),
			glm::vec3(0.8, 0.2, 1.0f),
			glm::vec3(0.8,0.2,0.2)));
	cross->addFunction(ADD,
		new Gaussian(0.370,
			glm::vec3(0.0, 0.0, 0),
			glm::vec3(0.0, 0.0, 0.78),
			glm::vec3(1.19, 0.155, 1.0f),
			glm::vec3(0.8,0.2,0.2)));


	Spot* gridSpot = new Spot();
	gridSpot->addFunction(ADD, new Gaussian(1.0, glm::vec3(0,0.1,0), glm::vec3(0.0), glm::vec3(1.0f / 4.29f, 0.25f / 4.29f, 1.0f)));
	gridSpot->addFunction(ADD, new Gaussian(1.0, glm::vec3(0,-0.1, 0), glm::vec3(0.0), glm::vec3(1.0f / 4.29f, 0.25f / 4.29f, 1.0f)));
	gridSpot->addFunction(ADD, new Gaussian(1.0, glm::vec3(0.1,0,0), glm::vec3(0.0), glm::vec3(0.25f / 4.29f, 1.0f / 4.29f, 1.0f)));
	gridSpot->addFunction(ADD, new Gaussian(1.0, glm::vec3(-0.1, 0, 0), glm::vec3(0.0), glm::vec3(0.25f / 4.29f, 1.0f / 4.29f, 1.0f)));
	

	Spot* modifiedGabor = new Spot();
	modifiedGabor->addFunction(ADD, new Gaussian(3.0, glm::vec3(-0.2,0,0), glm::vec3(0,0.0, 3.14 / 4.0), glm::vec3(1.0f / 4.29f, 0.2f / 4.29f, 1.0f)));
	modifiedGabor->addFunction(ADD, new Gaussian(3.0, glm::vec3(0.0, 0, 0), glm::vec3(0, 0.0, 3.14 / 4.0), glm::vec3(1.0f / 4.29f, 0.2f / 4.29f, 1.0f),glm::vec3(0.0f)));
	//modifiedGabor->getGaussiansVector()->at(1).scale.w = -modifiedGabor->getGaussiansVector()->at(0).getDeterminant() / 6.28f;
	modifiedGabor->addFunction(ADD, new Gaussian(3.0, glm::vec3(0.2, 0, 0), glm::vec3(0, 0.0, 3.14 / 4.0), glm::vec3(1.0f / 4.29f, 0.2f / 4.29f, 1.0f)));

	//modifiedGabor->addFunction(MULT, new Gaussian(-1.0, glm::vec3(0.0), glm::vec3(0.0), glm::vec3(0.5f / 4.29f, 0.5f / 4.29f, 1.0f)));
	//modifiedGabor->getGaussiansVector()->at(1).scale.w = modifiedGabor->getGaussiansVector()->at(1).getDeterminant() / 6.28f;

	Spot* harmonicSpot = new Spot();
	harmonicSpot->addFunction(ADD, new Harmonic(1.0f, glm::vec2(0.785, 0.0)));
	//harmonicSpot->addFunction(ADD, new ConstantParam(0.5));

	DistribParam* fullRandom = new DistribParam(
		glm::vec3(2.0,2.0,8),
		glm::vec2(0.0, 1.0),
		glm::mat3(glm::vec3(-0.5, -0.5, -0.5), glm::vec3(0.5), glm::vec3(0.5, 0.5, 0.5)),
		glm::vec4(0.0, 0.0, 0.0, 3.14),
		glm::vec3(1.0),
		glm::vec3(1.0));
	
	// ---- tissus bleu dans le papier du LRP
	int nbGaussiennes = 4;
	Spot* fabricBlueElement = new Spot(0.18);
	fabricBlueElement->addFunction(ADD, new Gaussian(1.0, glm::vec3(-0.235, 0.2, 0), glm::vec3(0, 0, -1.050f), glm::vec3(0.18, 0.040, 1.0f),glm::vec3(0.138,0.5,0.8)));
	fabricBlueElement->addFunction(ADD, new Gaussian(1.0, glm::vec3(0.235, 0.2, 0), glm::vec3(0, 0, 1.200f),glm::vec3(0.18, 0.050, 1.0f), glm::vec3(0.138, 0.5, 0.8)));
	// Test avec 4 gaussiennes
	fabricBlueElement->addFunction(ADD, new Gaussian(1.0, glm::vec3(-0.235, -0.2, 0), glm::vec3(0, 0, -1.050f), glm::vec3(0.18, 0.040, 1.0f), glm::vec3(0.138, 0.5, 0.8)));
	fabricBlueElement->addFunction(ADD, new Gaussian(1.0, glm::vec3(0.235, -0.2, 0), glm::vec3(0, 0, 1.200f), glm::vec3(0.18, 0.050, 1.0f), glm::vec3(0.138, 0.5, 0.8)));
	// Test avec 6 Gaussiennes
	//fabricBlueElement->addFunction(ADD, new Gaussian(100.0 / nbGaussiennes, glm::vec3(-0.235, -0.3, 0), glm::vec3(0, 0, -1.050f), glm::vec3(0.18, 0.040, 1.0f)));
	//fabricBlueElement->addFunction(ADD, new Gaussian(100.0 / nbGaussiennes, glm::vec3(0.235, -0.3, 0), glm::vec3(0, 0, 1.250f), glm::vec3(0.18, 0.060, 1.0f)));

	DistribParam* fabricBlueDistribution = new DistribParam(
		glm::vec3(2.0,2.0,8),
		glm::vec2(0.9, 0.9),
		glm::mat3(glm::vec3(-0.1,-0.5,-0.5), glm::vec3(0.5), glm::vec3(0.1,0.5,0.5)),
		glm::vec4(0.0, 0.0, 0.0, 3.14),
		glm::vec3(1.0),
		glm::vec3(1.0));
	// -----------------

	// ---- tissus jaune / or dans le LRP
	nbGaussiennes = 4;
	Spot* fabricYellowElement = new Spot(0.33f / nbGaussiennes);
	//fabricYellowElement->addFunction(ADD, new Gaussian(100.0 / nbGaussiennes, glm::vec3(-0.25, 0.25, 0), glm::vec3(0, 0, 0.0f), glm::vec3(0.15, 0.1, 1.0f)));
	//fabricYellowElement->addFunction(ADD, new Gaussian(-25.0 / nbGaussiennes, glm::vec3(0.25, 0.25, 0), glm::vec3(0, 0, 0.0f), glm::vec3(0.075, 0.1, 1.0f)));
	//// Test avec 4 gaussiennes
	//fabricYellowElement->addFunction(ADD, new Gaussian(100.0 / nbGaussiennes, glm::vec3(0.25, -0.25, 0), glm::vec3(0, 0, 0.0f), glm::vec3(0.15, 0.1, 1.0f)));
	//fabricYellowElement->addFunction(ADD, new Gaussian(-25.0 / nbGaussiennes, glm::vec3(-0.25, -0.25, 0), glm::vec3(0, 0, 0.0f), glm::vec3(0.075, 0.1, 1.0f)));

	fabricYellowElement->addFunction(ADD, 
		new Gaussian(
			100.0 / nbGaussiennes, 
			glm::vec3(-0.25, 0.385, 0), 
			glm::vec3(0, 0, 0.0f), 
			glm::vec3(0.15, 0.045, 1.0f)));
	fabricYellowElement->addFunction(ADD, 
		new Gaussian(
			100.0 / nbGaussiennes, 
			glm::vec3(-0.25, 0.15, 0), 
			glm::vec3(0, 0, 0.0f), 
			glm::vec3(0.15, 0.05, 1.0f)));
	fabricYellowElement->addFunction(ADD, 
		new Gaussian(
			25.0 / nbGaussiennes, 
			glm::vec3(0.3, 0.25, 0), 
			glm::vec3(0, 0, 0.0f), 
			glm::vec3(0.075, 0.1, 1.0f),
			glm::vec3(-1.0f)));

	// Test avec 4 gaussiennes
	fabricYellowElement->addFunction(ADD, 
		new Gaussian(
			100.0 / nbGaussiennes, 
			glm::vec3(0.25, -0.15, 0), 
			glm::vec3(0, 0, 0.0f), 
			glm::vec3(0.15, 0.05, 1.0f)));
	fabricYellowElement->addFunction(ADD, 
		new Gaussian(
			100.0 / nbGaussiennes, 
			glm::vec3(0.25, -0.35, 0), 
			glm::vec3(0, 0, 0.0f), 
			glm::vec3(0.15, 0.05, 1.0f)));
	fabricYellowElement->addFunction(ADD, 
		new Gaussian(
			25.0 / nbGaussiennes, 
			glm::vec3(-0.3, -0.25, 0), 
			glm::vec3(0, 0, 0.0f), 
			glm::vec3(0.075, 0.1, 1.0f), 
			glm::vec3(-1.0f)));

	DistribParam* fabricYellowDistribution = new DistribParam(
		glm::vec3(1.0),
		glm::vec2(0.0, 1.0),
		glm::mat3(glm::vec3(-0.1, -0.1, -0.5), glm::vec3(0.5), glm::vec3(0.1, 0.1, 0.5)),
		glm::vec4(0.0, 0.0, 0.0, 3.14),
		glm::vec3(1.0),
		glm::vec3(1.0));
	// -----------------
	
	// ---- tissus gris (En cours)
	nbGaussiennes = 3;
	Spot* fabricGreyElement = new Spot(0.33f / nbGaussiennes);
	fabricGreyElement->addFunction(ADD, new Gaussian(100.0 / nbGaussiennes, glm::vec3(-0.30, -0.25, 0), glm::vec3(0, 0, 0.0f), glm::vec3(0.05, 0.15, 1.0f)));
	fabricGreyElement->addFunction(ADD, new Gaussian(-100.0 / nbGaussiennes, glm::vec3(-0.15, -0.25, 0), glm::vec3(0, 0, 0.0f), glm::vec3(0.05, 0.15, 1.0f)));
	fabricGreyElement->addFunction(ADD, new Gaussian(100.0 / nbGaussiennes, glm::vec3(0.0, -0.25, 0), glm::vec3(0, 0, 0.0f), glm::vec3(0.05, 0.15, 1.0f)));
	// Test avec 6 gaussiennes
	fabricGreyElement->addFunction(ADD, new Gaussian(100.0 / nbGaussiennes, glm::vec3(0.0, 0.25, 0), glm::vec3(0, 0, -1.050f), glm::vec3(0.05, 0.15, 1.0f)));
	fabricGreyElement->addFunction(ADD, new Gaussian(-100.0 / nbGaussiennes, glm::vec3(0.15, 0.25, 0), glm::vec3(0, 0, -1.050f), glm::vec3(0.05, 0.15, 1.0f)));
	fabricGreyElement->addFunction(ADD, new Gaussian(100.0 / nbGaussiennes, glm::vec3(0.30, 0.25, 0), glm::vec3(0, 0, -1.050f), glm::vec3(0.05, 0.15, 1.0f)));
	DistribParam* fabricGreyDistribution = new DistribParam(
		glm::vec3(1.0),
		glm::vec2(0.0, 1.0),
		glm::mat3(glm::vec3(-0.1, -0.1, -0.5), glm::vec3(0.5), glm::vec3(0.1, 0.1, 0.5)),
		glm::vec4(0.0, 0.0, 0.0, 3.14),
		glm::vec3(1.0),
		glm::vec3(1.0));
	// -----------------
	
	// ----------- test de chargement de la liste de splat depuis une décomposition en gaussienne spectral (wavelet)
	//std::vector<GaussianSplat*> listFromWaveletAnalysis = GaussianSplat::readListFromFile((ressourceObjPath + "melon_test2_splatLevel_1.txt").c_str());
	//Spot* fabricSample = new Spot();
	//for (int i = 0; i < listFromWaveletAnalysis.size(); ++i) {
	//	float intensity = 0.2126 * listFromWaveletAnalysis[i]->color.x + 0.7152 * listFromWaveletAnalysis[i]->color.y + 0.0722 * listFromWaveletAnalysis[i]->color.z;// / 255.0f;
	//	fabricSample->addFunction(ADD, new Gaussian(
	//		( intensity / 255.0f),
	//		glm::vec3(-0.5, -0.5, 0.0) + glm::vec3(listFromWaveletAnalysis[i]->position, 0.0),
	//		glm::vec3(0, 0, listFromWaveletAnalysis[i]->rotation),
	//		glm::vec3(listFromWaveletAnalysis[i]->size, 1.0))
	//		);
	//}

	std::vector<Spot*> jeans;
	jeans.push_back(new Spot());
	jeans.back()->loadFromEllipsesFile((ressourceObjPath + "imageJean_2_grey_sub1_0_0.eli").c_str());

	jeans.push_back(new Spot());
	jeans.back()->loadFromEllipsesFile((ressourceObjPath + "imageJean_2_grey_sub1_0_1.eli").c_str());
	
	jeans.push_back(new Spot());
	jeans.back()->loadFromEllipsesFile((ressourceObjPath + "imageJean_2_grey_sub1_1_0.eli").c_str());
	
	jeans.push_back(new Spot());
	jeans.back()->loadFromEllipsesFile((ressourceObjPath + "imageJean_2_grey_sub1_1_1.eli").c_str());

	//jeans.push_back(new Spot());
	//jeans.back()->loadFromEllipsesFile((ressourceObjPath + "imageJean_2_grey_sub2_1_0.eli").c_str());
	//
	//jeans.push_back(new Spot());
	//jeans.back()->loadFromEllipsesFile((ressourceObjPath + "imageJean_2_grey_sub2_1_1.eli").c_str());
	//
	//jeans.push_back(new Spot());
	//jeans.back()->loadFromEllipsesFile((ressourceObjPath + "imageJean_2_grey_sub2_1_2.eli").c_str());
	//
	//jeans.push_back(new Spot());
	//jeans.back()->loadFromEllipsesFile((ressourceObjPath + "imageJean_2_grey_sub2_1_3.eli").c_str());




	Spot* jeansFabric = new Spot(0.5);
	//jeansFabric->loadFromEllipsesFile((ressourceObjPath + "imageJean_2_sub2_1_1.eli").c_str());
	jeansFabric->loadFromSVGParserResult((ressourceObjPath + "test_jeans.txt").c_str());

	Spot* spotCluster1 = new Spot();
	spotCluster1->loadFromSVGParserResult((ressourceObjPath + "fabric1.txt").c_str());
	//pré traitement pour réduire le nombre de gaussiennes :
	std::vector<Gaussian>* analyse = spotCluster1->getGaussiansVector();
	for (int i = 0; i < analyse->size(); ++i) {
		float greyAverage = analyse->at(i).color.r + analyse->at(i).color.g + analyse->at(i).color.b;
		greyAverage /= 3.0f;
		if (greyAverage < 0.15f) {
			spotCluster1->removeGaussianFunction(i);
			i--;
		}
	}

	std::vector<Spot*> melon;
	melon.push_back(new Spot());
	melon.back()->loadFromEllipsesFile((ressourceObjPath + "melon_C1_period_sub1_0_0_SLIC.eli").c_str());

	melon.push_back(new Spot());
	melon.back()->loadFromEllipsesFile((ressourceObjPath + "melon_C1_period_sub1_0_1_SLIC.eli").c_str());

	melon.push_back(new Spot());
	melon.back()->loadFromEllipsesFile((ressourceObjPath + "melon_C1_period_sub1_1_0_SLIC.eli").c_str());

	melon.push_back(new Spot());
	melon.back()->loadFromEllipsesFile((ressourceObjPath + "melon_C1_period_sub1_1_1_SLIC.eli").c_str());

	std::vector<Spot*> melon_color1;
	for (int x = 0; x < 4; ++x) 
		for (int y = 0; y < 4; ++y) {
			std::stringstream inputFile;
			inputFile << ressourceObjPath << "melon_C1_period_color1_sub2_" << x << "_"<<y<<"_SLIC.eli";
			std::cout << "Adding " << inputFile.str() << " in melon_color1" << std::endl;
			melon_color1.push_back(new Spot());
			melon_color1.back()->loadFromEllipsesFile(inputFile.str().c_str());
		}

	std::vector<Spot*> paint_peel;
	for (int x = 0; x < 4; ++x)
		for (int y = 0; y < 4; ++y) {
			std::stringstream inputFile;
			inputFile << ressourceObjPath << "test_C1_period_sub3_" << x << "_" << y << ".eli";
			paint_peel.push_back(new Spot());
			paint_peel.back()->loadFromEllipsesFile(inputFile.str().c_str());
		}
	
	

	for (int i = 0; i < paint_peel.size(); ++i) {
		std::vector<Gaussian>* temp = paint_peel[i]->getGaussiansVector();
		for (int j = 0; j < temp->size(); ++j) {
			bool enleveLaGrosse = false;
			bool enleveLaBlanche = false;
			enleveLaBlanche = 0.1 > glm::distance(temp->at(j).color, glm::vec3(165.0, 153.0, 147.0) / 255.0f);
			float minRadius = temp->at(j).scale.x > temp->at(j).scale.y ? temp->at(j).scale.y : temp->at(j).scale.x;
			enleveLaGrosse = minRadius > 0.15f;

			//float greyAverage = blue_test[i]->getGaussiansVector()->at(j).color.r + blue_test[i]-//etGaussiansVector()->at(j).color.g + blue_test[i]->getGaussiansVector()->at(j).color.b;
			//greyAverage /= 3.0f;
			if (enleveLaBlanche || enleveLaGrosse) {
				paint_peel[i]->removeGaussianFunction(j);
				j--;
			}
		}
	}
	
	Spot* fabric_test = new Spot();
	fabric_test->loadFromEllipsesFile((ressourceObjPath + "fabricBlue_zoom_greyEnhanced.eli").c_str());

	std::vector<Spot*> blue_fabric_test;
	for (int x = 0; x < 8; ++x)
		for (int y = 0; y < 8; ++y) {
			std::stringstream inputFile;
			inputFile << ressourceObjPath << "fabricBlue_zoom_seuil_sub3_" << x << "_" << y << ".eli";
			std::cout << "Adding " << inputFile.str() << " in blue_fabric_test" << std::endl;
			blue_fabric_test.push_back(new Spot());
			blue_fabric_test.back()->loadFromEllipsesFile(inputFile.str().c_str());
		}

	Spot* blackMetalSample = new Spot();
	blackMetalSample->loadFromEllipsesFile((ressourceObjPath + "Black-Metal-Texture_sample_SLIC.eli").c_str());
	

	
	//spotFromClustering->updateGaussianSpotColor(glm::vec3(0.0, 181.0f / 255.0f, 1.0f));

	DistribParam* clusterDistribution = new DistribParam(
		glm::vec3(1.0),
		glm::vec2(0.4, 0.6),
		glm::mat3(glm::vec3(-0.5, -0.5, -0.5), glm::vec3(0.5), glm::vec3(0.5, 0.5, 0.5)),
		glm::vec4(0.0, 0.0, 0.0, 3.14),
		glm::vec3(1.0),
		glm::vec3(1.0));

	// ------------------------------------- Loading pattern on GPU
	// Pour le moment, le choix du spot utilisé est mis par défaut sur le spot 0 (tout est en dur dans le shader de l'effet SpotNoise)
	std::vector<DistribParam*> distribList;
	std::vector<Spot*> spotList;
	spotList.push_back(cross);
	distribList.push_back(fullRandom);
	//spotList.push_back(blackMetalSample);
	//spotList.push_back(fabric_test);
	//distribList.push_back(clusterDistribution);
	//spotList.push_back(gridSpot);
	//spotList.push_back(TSpot);
	//spotList.push_back(testEllipseExtractor);
	// Tests en cours pour la peau de gargouille
	//distribList.push_back(fullRandom); // Controled Distribution (ID = 0)
	//spotList.push_back(patch1); // Spot (ID = 0)
	//spotList.push_back(patch2);
	//spotList.push_back(patch3);
	//spotList.push_back(patch4);
	//spotList.push_back(patch5);
	//spotList.push_back(patch6);

	// --------------------------------------------------------- LOUADINGUE 
	spotLoader = new GPUSpotVector(spotList,distribList);
	//spotLoader = new GPUSpotVector(blue_fabric_test, distribList);
	// -------------------------------------


	noiseOnMesh = new SurfacicSpotNoise("SpotNoise", int(sqrt(spotLoader->size())), 0, 3);
	// NODESET
	scene->getNode("Bunny")->setMaterial(noiseOnMesh);
	//scene->getNode("Dragon")->setMaterial(noiseOnMesh);

	noise = new SpotNoise("SpotNoiseTest",int(sqrt(spotLoader->size())),0,3);
	spotProfile = new SpotProfile("SpotProfileVisualizer",0);
	spotProfile2 = new SpotProfile("SpotProfileVisualizer2", 1);
	distribSpotProfile = new SpotProfile("DistribSpotVisualizer",0,false,true);
	distribution = new DistributionProfile("DistribVisualizer",0, int(sqrt(spotLoader->size())));


	//GPUTexture2D* dataField = new GPUTexture2D((ressourceObjPath + "rgb-compose.png"));
	GPUTexture2D* dataField = new GPUTexture2D((ressourceObjPath + "red.png"));
	noise->addDataField(dataField);
	noiseOnMesh->addDataField(dataField);
	distribution->addDataField(dataField);

	framebufferNoise = new GPUFBO("noiseRenderingTest");
	framebufferNoise->create(1024, 1024, 4, true, GL_RGBA32F, GL_TEXTURE_2D, 1);
	
	framebufferSpot = new GPUFBO("DisplaySpotProfile");
	framebufferSpot->create(256, 256, 1, false, GL_RGBA32F, GL_TEXTURE_2D, 1);
	
	framebufferSpotDistribution = new GPUFBO("DisplaySpotDistributionProfile");
	framebufferSpotDistribution->create(256, 256, 1, false, GL_RGBA32F, GL_TEXTURE_2D, 1);


	framebufferDistribution = new GPUFBO("DisplayDistributionProfile");
	framebufferDistribution->create(1024, 1024, 1, false, GL_RGBA32F, GL_TEXTURE_2D, 1);
	

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
	//return this->EngineGL::init();

	/*
	DistribParam dist1 = DistribParam(
	glm::vec3(1.0 / 8.0),
	glm::vec2(0.0, 1.0),
	glm::mat3(glm::vec3(-0.5),glm::vec3(0.5),glm::vec3(0.5)),
	glm::vec4(0.0,0.0,0.0,3.14),
	glm::vec3(1.0),
	glm::vec3(1.0));

	DistribParam dist2 = DistribParam(
	glm::vec3(1.0 / 8.0),
	glm::vec2(0.0, 1.0),
	glm::mat3(glm::vec3(-0.5), glm::vec3(0.5), glm::vec3(0.5)),
	glm::vec4(0.0, 0.0, 0.0, 1.57),
	glm::vec3(1.0),
	glm::vec3(1.0));

	DistribParam dist3 = DistribParam(
	glm::vec3(1.0 / 32.0),
	glm::vec2(0.0, 1.0),
	glm::mat3(glm::vec3(-0.5), glm::vec3(0.5), glm::vec3(0.5)),
	glm::vec4(0.0, 0.0, 0.0, 0.0),
	glm::vec3(1.0),
	glm::vec3(1.0));
	*/
	
}

void SampleEngine::render()
{
	// Begin Time query
	timeQuery->begin();

	// Clear Buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	

	float screenRatio = (float)w_Width / (float)w_Height;
	
	// Distribution results
	float x = 0;
	float profileSize = 0.2;
	
	// Resultat de distribution (voir l'effet DistributionProfile)
	// Profiles (effet SpotProfile)
	spotProfile->apply(NULL, framebufferSpot);
	framebufferSpot->display(glm::vec4(0.001, 0.001, profileSize, profileSize*screenRatio));
	distribSpotProfile->apply(NULL, framebufferSpotDistribution);
	//spotProfile2->apply(NULL, framebufferSpotDistribution);
	framebufferSpotDistribution->display(glm::vec4(profileSize, 0.001, profileSize, profileSize*screenRatio));
	distribution->apply(NULL, framebufferDistribution);
	framebufferDistribution->display(glm::vec4(0, 0.2*screenRatio, 0.4, 0.4*screenRatio));
	x += 0.4;
	profileSize *= 2;

	// FLAGRENDERING
	// Noise result (effet SpotNoise)
	noise->apply(NULL, framebufferNoise);

	//framebufferNoise->enable();
	//scene->camera()->setAspectRatio(1.0);
	//// Rendering every collected node
	//for (unsigned int i = 0; i < renderedNodes->nodes.size(); i++)
	//	renderedNodes->nodes[i]->render();
	//
	//
	//if (drawLights)
	//	lightingModel->renderLights();
	//
	//if (drawBoundingBoxes)
	//	for (unsigned int i = 0; i < renderedNodes->nodes.size(); i++)
	//		renderedNodes->nodes[i]->render(boundingBoxMat);
	//
	//framebufferNoise->disable();
	framebufferNoise->display(glm::vec4(x, 0.0, 0.6, 0.6*screenRatio), 0);
	
	//
	
	
	
	
	// end time Query					
	timeQuery->end();


	

	// scene has been rendered : no need for recomputing values until next camera/parameter change
	//Scene::getInstance()->camera()->setUpdate(true);
	scene->needupdate = false;

	this->record(this->activeRecord);

	spotLoader->reloadToGPU();
}

void SampleEngine::animate(const int elapsedTime)
{
	// TODO : reprendre le EngineGL comme exemple
	this->EngineGL::animate(elapsedTime);
}


extern "C" {
	char* toString(unsigned int n) {
		unsigned char nbdigit = 1;

		unsigned int copy = 0;
		for (copy = n; (copy /= 10) > 0 && n > 0; ++nbdigit);
		char* str = (char*)malloc(sizeof(char) * (nbdigit + 1));
		str[nbdigit] = '\0';

		unsigned char power = 0;
		for (copy = n; power < nbdigit; copy /= 10)
			str[nbdigit - power++ - 1] = (copy % 10) + '0';

		return str;
	}

	char* toFixString(unsigned int n, unsigned short charNumber) {
		char* str = (char*)malloc(sizeof(char) * (charNumber + 1));
		str[charNumber] = '\0';
		for (; charNumber > 0; n /= 10)
			str[--charNumber] = (n % 10) + '0';

		return str;
	}

}


static unsigned int currentFrameNumber = 0;
void SampleEngine::requestUpdate()
{
	if (!activeRecord){
		std::cout << "Begin capture at frame " << toFixString(currentFrameNumber, 5) << std::endl;
	} else { std::cout << "Stop capture at frame " << toFixString(currentFrameNumber, 5) << std::endl; }

	activeRecord = !activeRecord;

}


void SampleEngine::record(bool shouldI)
{
	if (shouldI) {
		std::stringstream frameName;
		DIR* captureFolder = opendir("./capture");
		if (captureFolder == NULL) {
			system("mkdir capture");
		} else closedir(captureFolder);

		frameName << "./capture/capture_" << toFixString(currentFrameNumber, 5) << ".ppm";
		this->dumpFramebufferToFile(frameName.str().c_str());
		++currentFrameNumber;
	}

}

void SampleEngine::dumpFramebufferToFile(const char * filename)
{
	static unsigned char* pixelsArray = NULL;
	static int previousWidth = getWidth();
	static int previousHeight = getHeight();
	static GLint viewPort[4];
	

	if (pixelsArray == NULL || previousWidth != getWidth() || previousHeight != getHeight()) {
		previousWidth = getWidth();
		previousHeight = getHeight();
		glGetIntegerv(GL_VIEWPORT, viewPort);
		if (pixelsArray != NULL) delete pixelsArray;
		pixelsArray = (unsigned char*)malloc(viewPort[2] * viewPort[3] * 3 * sizeof(unsigned char));
	}
	
	glReadPixels(0, 0, viewPort[2], viewPort[3], GL_RGB, GL_UNSIGNED_BYTE, pixelsArray);

	// RAW ppm writer
	FILE* tempImage = fopen(filename, "wb");
	if (tempImage != NULL) {
		fprintf(tempImage, "P6\n%d %d\n255\n", viewPort[2], viewPort[3]);
		fwrite(pixelsArray, sizeof(unsigned char), viewPort[2] * viewPort[3] * 3, tempImage);
		fclose(tempImage);
	}
	else std::cout << "Failed writing image " << filename << std::endl;


}
