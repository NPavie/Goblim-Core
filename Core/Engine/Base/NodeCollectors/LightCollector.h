/*
*	(c) XLim, UMR-CNRS
*	Authors: G.Gilet
*
*/

#ifndef _LIGHT_COLLECTOR_H
#define _LIGHT_COLLECTOR_H

#include "Engine/Base/NodeCollector.h"
#include <vector>


// Class Standard Collector


class LightCollector : public NodeCollector
{
protected:
	void collectRecur(Node* node);
public:
	LightCollector();
	~LightCollector();

	virtual void collect(Node* rootNode);



};

#endif