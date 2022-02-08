#ifndef _BOUNDINGBOX_H
#define _BOUNDINGBOX_H

// STL Includes
#include <iostream>
#include <glm/glm.hpp>
#include <vector>
#include "Engine/Base/Model.h"
class Node;
class BoundingBox
{
	public:
			
		BoundingBox(Node *n);
		~BoundingBox();
		
		glm::vec3 getMinValues();
		glm::vec3 getMaxValues();
		
		// compute the axis-aligned BB in world space of box in frame F
		void computeWorldAABB();

		void setModel(Model *m);
				

private :
	glm::vec3 minBox,maxBox;
	void compute();
	std::vector < glm::vec3 > listModelVertices;
	Node *node;


};
#endif