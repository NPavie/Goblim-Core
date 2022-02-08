/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */

#ifndef _STANDARD_COLLECTOR_H
#define _STANDARD_COLLECTOR_H

#include "Engine/Base/NodeCollector.h"
#include <vector>


// Class Standard Collector


class StandardCollector : public NodeCollector
{
	protected :
		void collectRecur(Node* node);
	public:
		StandardCollector();
		~StandardCollector();

		virtual void collect(Node* rootNode);



};

#endif
