/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */
#ifndef __Node_H
#define __Node_H

#include <glad/glad.h>
#include "Engine/Base/Frame.h"
#include "Model.h"
#include "Material.h"
#include "Animator.h"
#include <vector>
#include "Engine/Base/Lighting/ShadowModel.h" 
class BoundingBox;


class Node
{
	public:
		Node(std::string name);
		~Node();
		const std::string getName();

		bool loadModel(Model *m);

		// Render the Node and its sons using Node's material (or mat for overriding Node material)
		void render(Material *mat = NULL);

		void adopt(Node* son);
		bool disown(Node* son);

		Frame* frame();

		void setModel(Model *m);
		Model* getModel();

		void setMaterial(Material *r,bool recurse = false);
		Material* getMaterial();

		Animator* animator();
		void setAnimator(Animator *a);

		virtual void drawGeometry(int type=0);
		void animate(const int elapsedTime);

		std::vector<Node*> m_Sons;
		void displayLeaves(int level);

		ShadowModel* getShadowModel();

		//void computeAABB();

		//glm::vec3 boxSize;

		BoundingBox *box;

	protected:
		Model *m_Model;
		Material *m_Material;

		Frame *m_Frame;
		std::string m_Name;

		Animator *m_Animator;

		Node* m_Father;
		ShadowModel * m_Shadow;
		
};

#endif
