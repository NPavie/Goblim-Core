/*
*	(c) XLim, UMR-CNRS
*	Authors: G.Gilet
*
*/
#include "MeshNodeCollector.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Lighting/LightNode.hpp"

MeshNodeCollector::MeshNodeCollector()
{
}

MeshNodeCollector::~MeshNodeCollector()
{

}
void MeshNodeCollector::collect(Node* node)
{
	nodes.clear();
	//	nodes.push_back(node);

	collectRecur(node);
	//	printf("nodes : %d\n", nodes.size() );
}

void MeshNodeCollector::collectRecur(Node* node)
{
	if (node == NULL)
		return;

	if (dynamic_cast<LightNode* > (node) == NULL && node->getModel() != NULL) 
		nodes.push_back(node);

	for (std::vector<Node*>::iterator it = node->m_Sons.begin(); it != node->m_Sons.end(); ++it)
		collectRecur(*it);
}
