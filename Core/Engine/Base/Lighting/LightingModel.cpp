#include "LightingModel.h"
#include "Engine/Base/Scene.h"

LightingModel::LightingModel(std::string name,unsigned int ShadowTypeInfo)
{
	m_Name = name;
	needUpdate = true;
	collector = new LightCollector();
	ShadowType = ShadowTypeInfo;
}

LightingModel::~LightingModel()
{

}
void LightingModel::collect(Node *rootNode)
{
	collector->collect(rootNode);
}

void LightingModel::setCollector(NodeCollector *collector)
{
	if (collector != NULL)
		delete collector;
	this->collector = collector;
}

void LightingModel::update()
{
	for (vector< Node* >::iterator it = collector->nodes.begin(); it != collector->nodes.end(); ++it)
	{
		LightNode *lnode = dynamic_cast<LightNode*> (*it); 
		if (lnode != NULL && lnode->frame()->updateNeeded())
		{
			lnode->setPosition(glm::vec4(lnode->frame()->convertPtTo(glm::vec3(0.0),Scene::getInstance()->getRoot()->frame()),1.0));
			needUpdate = true;
		}
	}
}



std::vector<Node*> LightingModel::getNodes()
{
	return collector->nodes;
}