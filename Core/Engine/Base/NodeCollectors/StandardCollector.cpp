/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */
#include "StandardCollector.h"
#include "../Node.h"

StandardCollector::StandardCollector()
{
}

StandardCollector::~StandardCollector()
{
	
}
void StandardCollector::collect(Node* node)
{
	nodes.clear();
	//	nodes.push_back(node);

	collectRecur(node);
	//	printf("nodes : %d\n", nodes.size() );
}

void StandardCollector::collectRecur(Node* node)
{
	if(node == NULL)
		return;

	nodes.push_back(node);
	for (std::vector<Node*>::iterator it = node->m_Sons.begin(); it!=node->m_Sons.end(); ++it)
		collectRecur(*it);
}
