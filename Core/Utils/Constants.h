#ifndef _CONSTANTS_H
#define _CONSTANTS_H
#include <string>

const std::string rootPath = "./";


#define USE_ASSIMP
const std::string ressourceMaterialPath = rootPath+"Materials/";
const std::string ressourceEffectPath = rootPath+"Effects/";
const std::string ressourceObjPath = rootPath+"Objets/";
const std::string ressourceGPUPath = rootPath+"Resources/";
const std::string ressourceTexPath = rootPath+"Textures/";
const std::string kernelsPath = rootPath + "Kernels/";






#define HD 1

#if HD
#define FBO_WIDTH 1920
#define FBO_HEIGHT 1080
#else 
#define FBO_WIDTH 1280
#define FBO_HEIGHT 720
#endif


#endif
