/*
*	(c) XLim, UMR-CNRS
*	Authors: G.Gilet
*
*/
#include "Engine/Base/NodeCollectors/LightCollector.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Lighting/LightNode.hpp"

LightCollector::LightCollector()
{
}

LightCollector::~LightCollector()
{

}
void LightCollector::collect(Node* node)
{
	nodes.clear();
	//	nodes.push_back(node);

	collectRecur(node);
	//	printf("nodes : %d\n", nodes.size() );
}

void LightCollector::collectRecur(Node* node)
{
	if (node == NULL)
		return;

	if (dynamic_cast<LightNode* > (node) != NULL) 
		nodes.push_back(node);

	for (std::vector<Node*>::iterator it = node->m_Sons.begin(); it != node->m_Sons.end(); ++it)
		collectRecur(*it);
}
