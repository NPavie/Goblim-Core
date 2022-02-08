
#include "BoundingBox.h"
#include "Engine/Base/GeometricModel.h"
#include "Engine/Base/Frame.h"
#include "Engine/Base/Scene.h"
/*
*	Box' creation & destruction operations
*/
BoundingBox::BoundingBox(Node* n):
	node(n)
{

}
BoundingBox::~BoundingBox() {}

void BoundingBox::setModel(Model *m)
{
	listModelVertices = m->getGeometricModel()->box->getVertices();	
}

glm::vec3 BoundingBox::getMinValues()
{
	return minBox;
}

glm::vec3 BoundingBox::getMaxValues()
{
	return maxBox;
}

void BoundingBox::computeWorldAABB()
{
	glm::vec3 worldPoint;
	Frame *f = node->frame();
	
	if (listModelVertices.size() > 0)
	{
		for (int k = 0;k < 3;k++)
			{
				worldPoint = f->convertPtTo(listModelVertices.at(0), Scene::getInstance()->getRoot()->frame());
				minBox[k] = worldPoint[k];
				worldPoint = f->convertPtTo(listModelVertices.at(0), Scene::getInstance()->getRoot()->frame());
				maxBox[k] = worldPoint[k];
			}
	
		for (unsigned int i = 1; i < listModelVertices.size(); i++)
		{
		
			worldPoint = f->convertPtTo(listModelVertices.at(i), Scene::getInstance()->getRoot()->frame());
		
			for (int k = 0;k < 3;k++)
			{
				if (worldPoint[k] < minBox[k])
					minBox[k] = worldPoint[k];
				if (worldPoint[k] > maxBox[k])
					maxBox[k] = worldPoint[k];
			}
		}
	}

	// TODO : Compute AABB of sons and update accordingly
	/*
	std::vector <glm::vec3> listVertSons;
	for (unsigned int i = 0; i < node->m_Sons.size();i++)
	{
		listVertSons.push_back(node[i].box->getMaxValues());
		listVertSons.push_back(node[i].box->getMinValues());
	}
	for (unsigned int i = 0; i < listVertSons.size(); i++)
	{
		
		worldPoint = f->convertPtTo(listVertSons.at(i), Scene::getInstance()->getRoot()->frame());
		
		for (int k = 0;k < 3;k++)
		{
			if (worldPoint[k] < minBox[k])
				minBox[k] = worldPoint[k];
			if (worldPoint[k] > maxBox[k])
				maxBox[k] = worldPoint[k];
		}
	}
	*/



}

