/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */
#ifndef _GEOMETRIC_MODEL_H
#define _GEOMETRIC_MODEL_H

#include <glm/glm.hpp>
#include <vector>
#include <map>
#include "GPUResources/Buffers/GPUBuffer.h"
#include <glad/glad.h>
#include "Engine/Base/GeometricModelLoader.h"
#include "Engine/Base/BoundingBox/GeometricBoundingBox.h"

struct Face
{
	int s1,s2,s3;
};

struct Arete{
	int s1, s2;

	bool operator() (const Arete& x, const Arete& y) const 
	{
		if(x.s1 < y.s1) return true;
		else
		{
			if(x.s1 == y.s1) return (x.s2 < y.s2);
		}
		return false;
	}
};


class GeometricModel
{
	public:
		GeometricModel();
		GeometricModel(std::string name,bool loadnow = true);
		~GeometricModel();
		void buildSSBO();
		GPUBuffer* getSSBO(); 
		const std::string getName();

		int nb_vertex;
		int nb_faces;
		std::vector < glm::vec3 > listVertex;
		std::vector <Face> listFaces;
		std::vector <Face> listCoordFaces;
		std::vector <glm::vec3> listNormals;
		std::vector < glm::vec3 > listCoords;
		std::vector < glm::vec4 > listTangents;

		// For border vertex computation in open meshes : <s1, s2> <occurrence>
		std::map< Arete, int, Arete > listEdges;
		std::vector < float > listBorderTest;
		// Neighbours information : <Number of valid neighbours on border (Max 2), neighbour 1, neighbour 2>
		std::vector < glm::vec3 > listBorderNeighbours; 


		void createBoundingBox();
		GeometricBoundingBox *box;

		static GeometricModelLoader* loader;


	protected:
		std::string m_Name;
		GPUBuffer *m_Buffer;
		GLuint m_BindingPoint;

};


#endif
