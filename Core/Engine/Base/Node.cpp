/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */
#include "Engine/Base/Node.h"
#include <fstream>
#include <sstream>
#include <vector>
#include "Engine/Base/Scene.h"
#include "Engine/Base/BoundingBox/BoundingBox.h"

class Material;

Node::Node(std::string name)
{
	m_Name = name;
	m_Material = NULL;
	m_Model = NULL;
	m_Animator = NULL;
	m_Shadow = NULL;
	m_Model = NULL;

	// Frame Creation
	m_Frame = new Frame();

	box = new BoundingBox(this);


}

Node::~Node()
{
	LOG(TRACE) << "Deleting Node : " << m_Name ;

	delete m_Frame;
	//delete m_Animator;
	delete m_Shadow;
}

const std::string Node::getName()
{
	return(m_Name);
}

//void Node::computeAABB()
//{
	//if (m_Model != NULL)
	//	AABB.computeAABB(m_Model->getGeometricModel()->box,this->frame());
		
//}
void Node::setModel(Model *m)
{
	m_Model = m;
	m_Shadow = new ShadowModel(false,this);
	box->setModel(m);
}
Model* Node::getModel()
{
	return m_Model;
}
void Node::drawGeometry(int type)
{
	m_Model->drawGeometry(type);
}
void Node::setMaterial(Material *m,bool recurse)
{
	if (m_Model)
		m_Material = m;
	if (recurse)
		for(unsigned int i = 0;i < m_Sons.size();i++)
			m_Sons[i]->setMaterial(m,recurse);
}
Material* Node::getMaterial()
{
	return(m_Material);
}

void Node::render(Material* mat)
{
	if (m_Model)
    {
        if (mat)
        {
                mat->render(this);
        }
        else
        {
            if (m_Material != NULL) m_Material->render(this);
        }
    }
		
		
}

void Node::animate(const int elapsedTime)
{
	if(m_Animator)
	{
		m_Animator->animate(elapsedTime, frame()); // a definir
	}
	if (m_Material)
		m_Material->update(this,elapsedTime);

	//if (frame()->updateNeeded())
	//	this->box->computeWorldAABB();
}

void Node::adopt(Node* son)
{
	son->m_Father = this;
	son->frame()->attachTo(m_Frame);
	m_Sons.push_back(son);
}
Frame* Node::frame()
{
	return m_Frame;
}

void Node::setAnimator(Animator* a)
{
	m_Animator = a;
}

Animator* Node::animator()
{
	return m_Animator;
}


void Node::displayLeaves(int level)
{
	std::string toDisplay = std::string(" - " + m_Name);
	LOG(INFO)<< toDisplay;
	if (m_Sons.size() == 0)
	{
		LOG(INFO)<< ";" << endl;
	}
	else
	{
		int display_offset = level + (int)toDisplay.size();
		m_Sons[0]->displayLeaves(display_offset);
		if (m_Sons.size() > 1)
		{
			for (int i = 1; i < m_Sons.size(); i++)
			{
				for (int j = 0; j < display_offset; j++)
				{
					LOG(INFO) << " " ;
				}
				LOG(INFO)<< "|" << endl;

				for (int j = 0; j < display_offset; j++)
				{
					LOG(INFO)<< " ";
				}
				m_Sons[i]->displayLeaves(display_offset);
			}
		}
	}
}

bool Node::disown(Node* son)
{
	bool isInLeaves = false;
	for (int i = 0; i < m_Sons.size() && !isInLeaves; i++)
	{
		if (m_Sons[i] == son)
		{
			m_Frame->detach(son->frame());
			m_Sons.erase(m_Sons.begin() + i);
			return true;
		}
		else
		{
			isInLeaves = m_Sons[i]->disown(son);
		}
	}
	return isInLeaves;
}

ShadowModel *Node::getShadowModel()
{
	return m_Shadow;
}