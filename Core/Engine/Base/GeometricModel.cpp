/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */
#include <iostream>
#include "Engine/Base/GeometricModel.h"
#include "Engine/Base/GeometricModelLoader.h"
#include "Engine/Base/Scene.h"
using namespace std;


#ifdef USE_ASSIMP
#include "GeometricModelLoader/AssimpLoader.h"
GeometricModelLoader* GeometricModel::loader = new AssimpLoader();
#else
#include "GeometricModelLoader/OBJLoader.h"
GeometricModelLoader* GeometricModel::loader = new OBJLoader();
#endif





struct triangle{
    glm::vec4 A,B,C;
};

struct myPoly
{
    glm::vec4 point[5];
    glm::vec4 normal;
    glm::uvec4 info; //nP,support,pin,pout
    glm::uvec4 data;//visible,left,right
};

GeometricModel::GeometricModel()
{
    nb_vertex = 0;
    nb_faces = 0;
};
GeometricModel::GeometricModel(std::string name,bool loadnow)
{
    nb_vertex = 0;
    nb_faces = 0;
    m_Name = name;
	box = NULL;
    if (loadnow)
	{
		loader->loadModel(name,this);
		createBoundingBox();
	}
		
    
	
};
GeometricModel::~GeometricModel()
{
	if (box != NULL) delete box;
    
};
const std::string GeometricModel::getName()
{
    return m_Name;
}

GPUBuffer* GeometricModel::getSSBO()
{
    return m_Buffer;
} 
void GeometricModel::createBoundingBox()
{
	box = new GeometricBoundingBox(this);
}

void GeometricModel::buildSSBO()
{
    unsigned int* indiceRandom = new unsigned int [nb_faces];
	
	unsigned int key,key2,swap,x;
    //génération liste alléatoire pour mélanger les triangles
    for(unsigned int i=0;i<(unsigned int) nb_faces;i++)   
		indiceRandom[i] = i;

    x=nb_faces-1;
    //mélange des valeurs
  
    while(x!=0)
    {
        key                = (unsigned int) ( (double)(x) * (double)rand() / (1.+(double)RAND_MAX) );
        swap               = indiceRandom[x];
        indiceRandom[x]    = indiceRandom[key];
        indiceRandom[key]  = swap;
        x--;
    }

    //construction liste triangles en désordre

    //creation du buffer
   // GLError::enableCallback();
    m_Buffer = new GPUBuffer("Plucker"+m_Name);
    if(m_Buffer != NULL) 
		LOG(TRACE) << "GEOMETRIC MODEL:GPUbuffer création:"<< "liste triangle melangé"<<"nbr tri:"<<nb_faces;

    m_Buffer->create(nb_faces*sizeof(triangle)+sizeof(glm::mat4)+sizeof(glm::vec4),GL_SHADER_STORAGE_BUFFER ,GL_DYNAMIC_COPY);
    // copy des données
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_Buffer->getBuffer());
    //m_Buffer->bind();
    //data = (triangle*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, nb_faces*sizeof(triangle), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

    for( int i = 0;i<nb_faces;i++)
    {
        triangle tmp;

        tmp.A = glm::vec4(listVertex[listFaces[indiceRandom[i]].s1],1.0f);
        tmp.B = glm::vec4(listVertex[listFaces[indiceRandom[i]].s2],1.0f);
        tmp.C = glm::vec4(listVertex[listFaces[indiceRandom[i]].s3],1.0f);
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4)+i*sizeof(triangle)+sizeof(glm::vec4), sizeof(triangle), &tmp);
    }
    glm::vec4 tmp = glm::vec4((float)nb_faces);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, sizeof(glm::mat4), sizeof(glm::vec4), &tmp);

    //GLuint tmp =(GLuint) nb_faces;
    //glBufferSubData(GL_SHADER_STORAGE_BUFFER,0,sizeof(GLuint),&tmp);
    glFinish();
   // GLError::disableCallback();
    LOG(TRACE) <<"GEOMETRIC MODEL:fin de remplissage ssbo";
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	delete indiceRandom;
};
