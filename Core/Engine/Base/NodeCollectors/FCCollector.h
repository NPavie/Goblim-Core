/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet, G. Thoin
 *
 */

#ifndef _FRUSTUM_COLLECTOR_H
#define _FRUSTUM_COLLECTOR_H

#include "Engine/Base/NodeCollector.h"
#include <vector>
//#include <glm\gtc\matrix_access.hpp>
#include <glm/glm.hpp>

// Class Standard Collector
class Node;

class FCCollector : public NodeCollector
{
	protected :
		void collectRecur(Node* node);
	public:
		FCCollector();
		~FCCollector();

		virtual void collect(Node* rootNode);

		bool inFrustum(Node* node);
		void updateCam();

		glm::vec4 frustum[6];
};

#endif
