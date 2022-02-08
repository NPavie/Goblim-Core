#ifndef _GEOMETRICBOUNDINGBOX_H
#define _GEOMETRICBOUNDINGBOX_H

// STL Includes
#include <iostream>
#include <glm/glm.hpp>
#include <vector>

class GeometricModel;
class Frame;
class GeometricBoundingBox
{
	public:
			
		GeometricBoundingBox(GeometricModel* G);
		~GeometricBoundingBox();
		
		glm::vec3 getMinValues();
		glm::vec3 getMaxValues();
		std::vector<glm::vec3> getVertices();

		
		void compute(const glm::vec3 & minval, const glm::vec3 & maxval);

private :
	std::vector < glm::vec3 > vertices;
	glm::vec3 minBox,maxBox;
	GeometricModel *model;
	
	void compute();


};
#endif