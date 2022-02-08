
#ifndef _LIGHTING_MODEL_
#define _LIGHTING_MODEL_
#include "LightNode.hpp"
#include "Engine/Base/NodeCollectors/LightCollector.h"

#define NONE 			0
#define SHADOW_MAP 		1
#define SHADOW_VOLUME 	2
#define SHADOW_PLUCKER 	3




class LightingModel 
{

public :
	LightingModel(std::string name = "",unsigned int ShadowTypeInfo = NONE);
	~LightingModel();

	void collect(Node *rootNode);
	void setCollector(NodeCollector *collector);
	std::vector<Node*> getNodes();

	virtual void update();
	
	
protected:
	std::string m_Name;	
	NodeCollector* collector;
	bool needUpdate;
	unsigned int ShadowType;
};

#endif
	
