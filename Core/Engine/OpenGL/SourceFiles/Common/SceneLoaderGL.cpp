#include "Engine/OpenGL/SceneLoaderGL.h"

#include "Engine/Base/Scene.h"
#include <glm/gtc/matrix_transform.hpp> 


SceneLoaderGL::SceneLoaderGL()
{
}

SceneLoaderGL::~SceneLoaderGL()
{
}

Node* SceneLoaderGL::loadScene(string filename, bool SmoothNormal)
{
	gobScene = Scene::getInstance();
	AssimpLoader modelLoader;
	unsigned int normalMode;
	if(SmoothNormal) 
		normalMode =  aiProcess_GenSmoothNormals;
	else 
		normalMode =  aiProcess_GenNormals ;
	Node* node;
	// Create an instance of the Importer class
	Assimp::Importer importer;

	
	// Light definition
	
	LOG(INFO) << "Loading Scene File : " << filename ;

	const aiScene* scene = importer.ReadFile( filename,
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_FindDegenerates | 
			aiProcess_FindInvalidData  | 
			normalMode |
			 
			aiProcess_FindInstances );


	// If the import failed, report it
	if( !scene)
	{
		LOG(WARNING) << string(importer.GetErrorString()) ;
		throw std::logic_error(string("ERROR : Assimp Scene Loader: ") + filename + string(" could not be opened.\n") + string(importer.GetErrorString()));
		return NULL;
	}
	



	node = parseNodes(scene->mRootNode, scene);

	if (!scene->HasLights())	// if scene has no light create default white light in 0,10,0 and add it to parent node
		addDefaultLight(node);
	
	if (scene->HasCameras())
	{
		for (int i = 0 ;i < scene->mNumCameras;i++)
		loadCamera(scene->mRootNode,scene->mCameras[i]);
	}

	

	
return node;
}

LightNode* SceneLoaderGL::createLightNode(aiNode* node,aiLight* light) // TODO : add support for more complex light
{

	LightMaterial* lmat = new LightMaterial("LightMaterial-"+ string(light->mName.C_Str()),glm::vec4(light->mColorDiffuse.r,light->mColorDiffuse.g,light->mColorDiffuse.b,1.0));
	LightNode* lnode = new LightNode(node->mName.C_Str());
	gobScene->m_Nodes.insert(lnode->getName(),lnode);		// insert light node into node manager

	aiNode *tn = node;
	aiMatrix4x4 res;
	while (tn != NULL)
	{
		res = tn->mTransformation * res ;
		tn = tn->mParent;
	}
	
	glm::mat4 transfo = convertFromAiMatrix(res);
	lnode->frame()->setUpFromMatrix(transfo);
	lnode->setModel(gobScene->getModel<ModelGL>(ressourceObjPath+"Sphere.obj"));
	lnode->setMaterial(lmat);


	glm::vec3 pos = glm::vec3(light->mPosition.x,light->mPosition.y,light->mPosition.z);
	

	lnode->frame()->translate(glm::vec3(pos));
	lnode->frame()->scale(glm::vec3(10.0,10.0,10.0));

	lnode->setColor(glm::vec4(light->mColorDiffuse.r, light->mColorDiffuse.g, light->mColorDiffuse.b, 1.0));
	//gestion des type de lumière
	lnode->setPosition(glm::vec4(pos,0));
	if(light->mType == aiLightSource_DIRECTIONAL)
	{
		lnode->setType(DIRECTIONAL_LIGHT);
		lnode->setDirection(glm::vec4(light->mDirection.x,light->mDirection.y,light->mDirection.z,0	));
	}
	else if(light->mType == aiLightSource_SPOT)
	{
		lnode->setType(SPOT_LIGHT);
		lnode->setDirection(glm::vec4(light->mDirection.x,light->mDirection.y,light->mDirection.z,0	));
		lnode->setAngle(light->mAngleOuterCone);		
	}
	else
	{
		lnode->setType(POINT_LIGHT);
	}
	return lnode;
}

void SceneLoaderGL::addDefaultLight(Node* targetnode)
{
	
		LOG(INFO) << "No Light in scene, adding default white light";

		LightMaterial* lmat = new LightMaterial("LightMaterial-"+ string("DefaultLight"),glm::vec4(0.8));
		LightNode* lnode = new LightNode("DefaultLight");
		gobScene->m_Nodes.insert(lnode->getName(),lnode);		// insert light node into node manager

		lnode->setModel(gobScene->getModel<ModelGL>(ressourceObjPath+"Sphere.obj"));
		lnode->setMaterial(lmat);

		targetnode->adopt(lnode);

		lnode->frame()->translate(glm::vec3(0.0,10.0,0.0));
		lnode->frame()->scale(glm::vec3(10.0,10.0,10.0));
		lnode->setPosition(glm::vec4(0.0,10.0,0.0,0.0));
		lnode->setType(SPOT_LIGHT);

}

void SceneLoaderGL::loadCamera(aiNode* n,aiCamera*  cam)
{
	LOG(TRACE) << "Loading camera " << cam->mName.C_Str();
	CameraGL* g_Cam = gobScene->m_Cameras.get(cam->mName.C_Str());
		
	g_Cam->setFoV(180.0f*(cam->mHorizontalFOV)/3.1416f);	
	g_Cam->setZfar(cam->mClipPlaneFar);
	g_Cam->setZnear(cam->mClipPlaneNear + 0.01f);
		

	/************************************************************************** Collada format ***************************/
	//not used : aiMatrix4x4 matV = n->FindNode(cam->mName)->mTransformation;

	aiNode *tn = n->FindNode(cam->mName);
	aiMatrix4x4 res;
	while (tn != NULL)
	{
		res = tn->mTransformation * res ;

		tn = tn->mParent;
	}

	
	aiVector3D ai_center = cam->mPosition;
	aiVector3D ai_eye = cam->mLookAt;
	aiVector3D ai_up = cam->mUp;
	
	glm::mat4 transfo = convertFromAiMatrix(res);
	glm::vec4 center = transfo * glm::vec4(ai_center.x,ai_center.y,ai_center.z,1.0);
	glm::vec4 eye = transfo * glm::vec4(ai_eye.x,ai_eye.y,ai_eye.z,0.0);
	glm::vec4 up = transfo * glm::vec4(ai_up.x,ai_up.y,ai_up.z,0.0);
	
	//glm::mat4 up = glm::lookAt(from,glm::vec4(ai_up.x,ai_up.y,ai_up.z) ,glm::vec3(0.0,1.0,0.0));
	g_Cam->lookAt(glm::vec3(eye+center),glm::vec3(center),glm::vec3(up));
		//setUpFromMatrix(tou);


/************************************************************************** 3DS Format Conversion
	glm::mat4 trot;
	trot = glm::rotate(trot,90.0f,glm::vec3(1.0,0.0,0.0));
	glm::vec4 tto = trot * glm::vec4(t.x,t.y,t.z,0.0);
	glm::vec4 tfrom =  trot * glm::vec4(tr.x,tr.y,tr.z,1.0);
	glm::vec4 tup =  trot * glm::vec4(up.x,up.y,up.z,0.0);

	glm::vec3 to =glm::vec3(tto.x,tto.y,tto.z);
	glm::vec3 from = glm::vec3(tfrom.x,tfrom.y,tfrom.z);
	glm::vec3 pTo = from + 100.0f * to;


	gobScene->camera()->lookAt(glm::vec3(pTo.x,pTo.y,pTo.z) ,from,glm::vec3(up.x,up.y,up.z));

	*/
}


Node* SceneLoaderGL::createStandardNode(aiNode* node,const aiScene* scene)
{
	Node* g_Node = NULL;
	g_Node = gobScene->getNode(node->mName.C_Str());
	g_Node->frame()->setUpFromMatrix(convertFromAiMatrix(node->mTransformation));

	for(int i = 0;i <  (int) node->mNumMeshes ;i++)
	{
		g_Node->setModel(parseModel(scene->mMeshes[node->mMeshes[i]]));
		g_Node->setMaterial(parseMaterial(scene->mMaterials[scene->mMeshes[node->mMeshes[i]]->mMaterialIndex]));
	}
	return g_Node;
}




Node* SceneLoaderGL::parseNodes(aiNode* node,const aiScene* scene)
{
	aiNode* ai_node = NULL;
	// check if node is a Light
	int i =0;
	while (i < (int) scene->mNumLights && node-> mName != scene->mLights[i]->mName)
		i++;

	if (i < (int) scene->mNumLights)
		ai_node = node;
	
	Node* g_Node = NULL;


	if (ai_node != NULL) // if node is a light, create and define new light
		g_Node = createLightNode(ai_node,scene->mLights[i]);
	else		// else create standard node
		g_Node = createStandardNode(node,scene);

	for(int i = 0;i < (int) node->mNumChildren;i++)
		g_Node->adopt(parseNodes(node->mChildren[i],scene));

	return g_Node;
}

Model* SceneLoaderGL::parseModel(aiMesh* mesh)
{
	AssimpLoader modelLoader;
	ModelGL* modelGL;

    LOG(INFO)<< "Loading model " << mesh->mName.C_Str() ;
	modelGL = new ModelGL(mesh->mName.C_Str(),false);
	modelLoader.loadFromMesh(mesh,modelGL->getGeometricModel());
	modelGL->loadToGPU();
	gobScene->m_Models.insert(modelGL->name(),modelGL);
	

	return modelGL;

}

Material* SceneLoaderGL::parseMaterial(aiMaterial* mat)
{
	
	
	// IS Texture Based
	
	
//		aiString path;
//		
//		 if (AI_SUCCESS == mat->GetTexture(aiTextureType_DIFFUSE,0,&path))
//		 {
//			GPUTexture2D *t = gobScene->getResource<GPUTexture2D> (path.C_Str());
//			TextureMaterial* tmat = new TextureMaterial("TMat",t);
//			if (AI_SUCCESS == mat->GetTexture(aiTextureType_HEIGHT ,0,&path))
//			{
//				tmat->setNormalMap(gobScene->getResource<GPUTexture2D> (path.C_Str()));
//			}
//			/*
//			std::cout << "Diffuse : " << mat->GetTextureCount(aiTextureType_DIFFUSE)  << std::endl;
//			std::cout << "Height : " << mat->GetTextureCount(aiTextureType_HEIGHT)  << std::endl;
//			std::cout << "Normals : " << mat->GetTextureCount(aiTextureType_NORMALS)  << std::endl;
//			std::cout << "Displacement : " << mat->GetTextureCount(aiTextureType_DISPLACEMENT)  << std::endl;
//			std::cout << "Unknown : " << mat->GetTextureCount(			aiTextureType_UNKNOWN
//)  << std::endl;
//			*/
//
//
//			return(tmat);
//		 }
//	
	
	ColorMaterial *mater = new ColorMaterial("Color",glm::vec4(0.6,0.6,0.6,1.0));
	return mater;
}

glm::mat4 SceneLoaderGL::convertFromAiMatrix(aiMatrix4x4 a)
{
	a = a.Transpose();
	glm::mat4 m;
	m[0][0] = a.a1;
	m[0][1] = a.a2;
	m[0][2] = a.a3;
	m[0][3] = a.a4;
	m[1][0] = a.b1;
	m[1][1] = a.b2;
	m[1][2] = a.b3;
	m[1][3] = a.b4;
	m[2][0] = a.c1;
	m[2][1] = a.c2;
	m[2][2] = a.c3;
	m[2][3] = a.c4;
	m[3][0] = a.d1;
	m[3][1] = a.d2;
	m[3][2] = a.d3;
	m[3][3] = a.d4;

	return m;
}
