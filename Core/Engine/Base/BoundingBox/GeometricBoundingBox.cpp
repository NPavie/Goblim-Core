
#include "Engine/Base/BoundingBox/GeometricBoundingBox.h"
#include "Engine/Base/GeometricModel.h"
#include "Engine/Base/Frame.h"
#include "Engine/Base/Scene.h"
/*
*	Box' creation & destruction operations
*/
GeometricBoundingBox::GeometricBoundingBox(GeometricModel* G):
	model(G)
{
	if (model != NULL)
		compute();
}
GeometricBoundingBox::~GeometricBoundingBox() {}
void GeometricBoundingBox::compute()
{

	for (int k = 0;k < 3;k++)
		{
			minBox[k] = model->listVertex.at(0)[k];
			maxBox[k] = model->listVertex.at(0)[k];
		}



	for (unsigned int i = 0; i < model->listVertex.size(); i++)
	{
		
		glm::vec3 point = model->listVertex.at(i);
		
		for (int k = 0;k < 3;k++)
		{
			if (point[k] < minBox[k])
				minBox[k] = point[k];
			if (point[k] > maxBox[k])
				maxBox[k] = point[k];
		}
	}
	vertices.push_back(glm::vec3(minBox[0],minBox[1],minBox[2]));
	vertices.push_back(glm::vec3(minBox[0],minBox[1],maxBox[2]));
	vertices.push_back(glm::vec3(minBox[0],maxBox[1],maxBox[2]));
	vertices.push_back(glm::vec3(minBox[0],maxBox[1],minBox[2]));

	vertices.push_back(glm::vec3(maxBox[0],maxBox[1],maxBox[2]));
	vertices.push_back(glm::vec3(maxBox[0],maxBox[1],minBox[2]));
	vertices.push_back(glm::vec3(maxBox[0],minBox[1],minBox[2]));
	vertices.push_back(glm::vec3(maxBox[0],minBox[1],maxBox[2]));
	


}

void GeometricBoundingBox::compute(const glm::vec3 & minval, const glm::vec3 & maxval)
{
	minBox = minval;
	maxBox = maxval;
	
	vertices.push_back(glm::vec3(minBox[0],minBox[1],minBox[2]));
	vertices.push_back(glm::vec3(minBox[0],minBox[1],maxBox[2]));
	vertices.push_back(glm::vec3(minBox[0],maxBox[1],maxBox[2]));
	vertices.push_back(glm::vec3(minBox[0],maxBox[1],minBox[2]));

	vertices.push_back(glm::vec3(maxBox[0],maxBox[1],maxBox[2]));
	vertices.push_back(glm::vec3(maxBox[0],maxBox[1],minBox[2]));
	vertices.push_back(glm::vec3(maxBox[0],minBox[1],minBox[2]));
	vertices.push_back(glm::vec3(maxBox[0],minBox[1],maxBox[2]));
	


}


glm::vec3 GeometricBoundingBox::getMinValues()
{
	return minBox;
}

glm::vec3 GeometricBoundingBox::getMaxValues()
{
	return maxBox;
}

std::vector <glm::vec3> GeometricBoundingBox::getVertices()
{
	return vertices;
}
/*
void GeometricBoundingBox::computeWorldAABB(Frame *f)
{
	glm::vec3 worldPoint;

	for (int k = 0;k < 3;k++)
		{
			worldPoint = f->convertPtTo(vertices.at(0), Scene::getInstance()->getRoot()->frame());
			minBox[k] = worldPoint[k];
			worldPoint = f->convertPtTo(vertices.at(0), Scene::getInstance()->getRoot()->frame());
			maxBox[k] = worldPoint[k];
		}
	
	for (unsigned int i = 1; i < vertices.size(); i++)
	{
		
		worldPoint = f->convertPtTo(vertices.at(i), Scene::getInstance()->getRoot()->frame());
		
		for (int k = 0;k < 3;k++)
		{
			if (worldPoint[k] < minBox[k])
				minBox[k] = worldPoint[k];
			if (worldPoint[k] > maxBox[k])
				maxBox[k] = worldPoint[k];
		}
	}



}
*/
