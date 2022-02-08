/*
*	(c) XLim, UMR-CNRS
*	Authors: G.Gilet
*
*/

#ifndef _MESHNODE_COLLECTOR_H
#define _MESHNODE_COLLECTOR_H

#include "Engine/Base/NodeCollector.h"
#include <vector>


// Class MeshNode Collector : will collect nodes associated to a mesh, excluding LightNodes


class MeshNodeCollector : public NodeCollector
{
protected:
	void collectRecur(Node* node);
public:
	MeshNodeCollector();
	~MeshNodeCollector();

	virtual void collect(Node* rootNode);



};

#endif