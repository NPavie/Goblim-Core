/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */

#ifndef _NODE_COLLECTOR_H
#define _NODE_COLLECTOR_H

#include <vector>

// Class Node Collector
class Node;


class NodeCollector
{
	public:
		NodeCollector(){};
		virtual ~NodeCollector(){};

		virtual void collect(Node* rootNode){};

		std::vector<Node*> nodes;


};

#endif
