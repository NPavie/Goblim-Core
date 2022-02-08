/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */
#include "FCCollector.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Scene.h"
#include <omp.h>
#include "Engine/Base/BoundingBox/BoundingBox.h"
#include <glm/gtc/matrix_access.hpp>

FCCollector::FCCollector()
{	
}

FCCollector::~FCCollector() {}


void FCCollector::collect(Node* node)
{
	nodes.clear();

	if (Scene::getInstance()->camera()->needUpdate())
		this->updateCam();

	if (node->getName() == "Root")
	{
		int i = 0;
		#pragma omp parallel //num_threads(4)
		{
			#pragma omp for schedule(dynamic, glm::max((int)node->m_Sons.size() / omp_get_num_threads(), 1))
			for (i = 0; i < (int)node->m_Sons.size(); i++)
				collectRecur(node->m_Sons.at(i));
			//#pragma omp barrier
		}
	}
}
void FCCollector::collectRecur(Node* node)
{
	if (node == NULL)
		return;
	
	if (node->getModel() != NULL)
		if (inFrustum(node))
			nodes.push_back(node);
	for (std::vector<Node*>::iterator it = node->m_Sons.begin(); it != node->m_Sons.end(); ++it)
		collectRecur(*it);
	
}
bool FCCollector::inFrustum(Node* node)
{
	
	glm::vec3 min_val = node->box->getMinValues();
	glm::vec3 max_val = node->box->getMinValues();
	for (int i = 0; i < 6; i++)
	{
		if (glm::dot(frustum[i],glm::vec4(min_val,1.0))>0)
			continue;
		if (glm::dot(frustum[i],glm::vec4(max_val.x,min_val.y,min_val.z,1.0))>0)
			continue;
		if (glm::dot(frustum[i],glm::vec4(min_val.x,max_val.y,min_val.z,1.0))>0)
			continue;
		if (glm::dot(frustum[i],glm::vec4(max_val.x,max_val.y,min_val.z,1.0))>0)
			continue;
		if (glm::dot(frustum[i],glm::vec4(min_val.x,min_val.y,max_val.z,1.0))>0)
			continue;
		if (glm::dot(frustum[i],glm::vec4(max_val.x,min_val.y,max_val.z,1.0))>0)
			continue;
		if (glm::dot(frustum[i],glm::vec4(min_val.x,max_val.y,max_val.z,1.0))>0)
			continue;
		if (glm::dot(frustum[i],glm::vec4(max_val.x,max_val.y,max_val.z,1.0))>0)
			continue;
		return false;
	}
	return true;
}

void FCCollector::updateCam()
{
	float length;
	//glm::mat4 proj = Scene::getInstance()->camera()->getProjectionMatrix() * (*Scene::getInstance()->camera()->getFrame()->getMatrix());
	glm::mat4 proj = Scene::getInstance()->getRoot()->frame()->getTransformMatrix();
		//Scene::getInstance()->camera()->getProjectionMatrix() 
		//* glm::inverse(Scene::getInstance()->camera()->getModelViewMatrix(Scene::getInstance()->getRoot()->frame())) 
		//* (*Scene::getInstance()->camera()->getFrame()->getMatrix()) 
		//* (Scene::getInstance()->camera()->getFrame()->rootMatrix);

	glm::vec4 rowX = glm::row(proj, 0);
	glm::vec4 rowY = glm::row(proj, 1);
	glm::vec4 rowZ = glm::row(proj, 2);
	glm::vec4 rowW = glm::row(proj, 3);

	frustum[0] = (rowW + rowX);
	frustum[1] = (rowW - rowX);
	frustum[2] = (rowW + rowY);
	frustum[3] = (rowW - rowY);
	frustum[4] = (rowW + rowZ);
	frustum[5] = (rowW - rowZ);

	// Normalizing the planes.
	for (int i = 0; i < 6; i++)
	{
		frustum[i] = glm::normalize(frustum[i]);
	//	glm::vec3 normal(frustum[i].x, frustum[i].y, frustum[i].z);
		//length = glm::length(normal);
		//frustum[i] = (frustum[i] / length);
	}
}