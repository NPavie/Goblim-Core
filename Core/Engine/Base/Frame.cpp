/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */
#include <glm/gtc/matrix_transform.hpp> 
#include "Engine/Base/Frame.h"
#include <iostream>

Frame::Frame()
{
	reference = NULL;
	matrix = glm::mat4(1.0f);
	isCamera = false;
}

void Frame::attachTo(Frame *f)
{
	reference = f;
	if (f != NULL)
		f->m_Sons.push_back(this);

}
glm::mat4 Frame::getTransformMatrix()
{
	if (reference != NULL)
		return(reference->getTransformMatrix() * matrix);
	else
		return(matrix);
}
glm::mat4 Frame::getRootMatrix()
{
	if (reference != NULL && !reference->isCameraFrame())
		return(reference->getRootMatrix() * matrix);
	else
		return(matrix);
}

glm::mat4 Frame::getMatrixCopy()
{
	return (matrix);
}

Frame::~Frame()
{

}

void Frame::setUpFromMatrix(const glm::mat4 & m)
{
	matrix = m;
	setUpdate(true);
}

void Frame::loadIdentity()
{
	matrix = glm::mat4(1.0f);

		setUpdate(true);
}

// Transformations géometriques

void Frame::rotate(glm::vec3 axis, float angle)
{
	matrix = glm::rotate(matrix,angle,axis);
	setUpdate(true);
}
void Frame::translate(glm::vec3 t)
{
	matrix = glm::translate(matrix,t);
	setUpdate(true);

}

void Frame::scale(glm::vec3 s)
{
	matrix = glm::scale(matrix,s);
	setUpdate(true);
}


bool Frame::isCameraFrame()
{
	return isCamera;
}
void Frame::setAsCameraFrame(bool r)
{
	isCamera = r;
}

glm::vec3 Frame::convertPtFrom(glm::vec3 pt,Frame *from)
{
	glm::vec4 pInRoot = from->getTransformMatrix() * glm::vec4(pt,1.0);
	glm::vec4 ret = glm::inverse(getTransformMatrix()) * pInRoot;
	return (glm::vec3(ret));
}
glm::vec3 Frame::convertDirFrom(glm::vec3 dir,Frame *from)
{
	glm::vec4 pInRoot = from->getTransformMatrix() * glm::vec4(dir,0.0);
	glm::vec4 ret = glm::inverse(getTransformMatrix()) * pInRoot;
	return (glm::vec3(ret));
}
glm::vec3 Frame::convertPtTo(glm::vec3 pt,Frame *to)
{
	glm::vec4 pInRoot = getTransformMatrix() * glm::vec4(pt,1.0);
	glm::vec4 ret = glm::inverse(to->getTransformMatrix()) * pInRoot;
	return (glm::vec3(ret));
}
glm::vec3 Frame::convertDirTo(glm::vec3 dir,Frame *to)
{
	glm::vec4 pInRoot = getTransformMatrix() * glm::vec4(dir,0.0);
	glm::vec4 ret = glm::inverse(to->getTransformMatrix()) * pInRoot;
	return (glm::vec3(ret));
}
glm::vec3 Frame::getScreenPosition(glm::vec3 v)
{
	glm::vec4 pos = getTransformMatrix() * glm::vec4(v,1.0);
	pos /= pos.w;
	return(glm::vec3(pos));
}

void Frame::setUpdate(bool t)
{
	m_ToUpdate = t;
	
	if (t && !isCameraFrame())
	{
		for(int i = 0;i < m_Sons.size();i++)
			m_Sons[i]->setUpdate(t);
	}
	
}
bool Frame::updateNeeded()
{
	return(m_ToUpdate);
}

bool Frame::detach(Frame* f)
{
	bool isInLeaves = false;
	for (int i = 0; i < f->m_Sons.size() && !isInLeaves; i++)
	{
		if (m_Sons[i] == f)
		{
			m_Sons.erase(m_Sons.begin() + i);
			return true;
		}
		else
		{
			isInLeaves = m_Sons[i]->detach(f);
		}
	}
	return isInLeaves;
}