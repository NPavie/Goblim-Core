/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */


#include "Utils/Constants.h"

#ifdef USE_ASSIMP

#ifndef _SCENELOADERGL_H
#define _SCENELOADERGL_H

#include "Engine/Base/GeometricModelLoader/AssimpLoader.h"
#include <GL/glew.h>
#include "Engine/Base/Node.h"
#include "Engine/OpenGL/ModelGL.h"
#include "Engine/Base/Scene.h"
#include "Materials/ColorMaterial/ColorMaterial.h"

#include "Materials/TextureMaterial/TextureMaterial.h"
#include "Materials/ColorMaterial/ColorMaterial.h"
#include "Materials/NormalMaterial/NormalMaterial.h"
#include "Materials/PhongMaterial/PhongMaterial.h"
#include "Materials/LightMaterial/LightMaterial.h"
#include "Engine/Base/Animators/RotationAnimator.h"


class SceneLoaderGL 
{
	public:
		SceneLoaderGL();
		~SceneLoaderGL();
		Node* loadScene(string filename,bool SmoothNormal=  true );


private :
	Node* parseNodes(aiNode* node,const aiScene* scene);
	Material* parseMaterial(aiMaterial* mat);
	Model* parseModel(aiMesh* mesh);
	glm::mat4 convertFromAiMatrix(aiMatrix4x4 a);
	void loadCamera(aiNode* n,aiCamera*  cam);
	LightingModelGL* parseLights(aiScene* scene);
	void addDefaultLight(Node* targetnode);
	LightNode* createLightNode(aiNode* node,aiLight* light);
	Node* createStandardNode(aiNode* node,const aiScene* scene);
	ModelGL* seekSimilarModel(aiMesh* mesh);
	Scene *gobScene;


};


#endif
#endif
