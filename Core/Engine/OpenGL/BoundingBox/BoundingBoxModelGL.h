/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */
#ifndef _BOUNDINGBOXMODELGL_H
#define _BOUNDINGBOXMODELGL_H


#include <GL/glew.h>
#include "Engine/Base/BoundingBox/GeometricBoundingBox.h"

class ModelGL;


class BoundingBoxModelGL 
{
	public:
		BoundingBoxModelGL(GeometricBoundingBox *box);
		~BoundingBoxModelGL();
		virtual void drawGeometry();
		

	protected:
		// Buffers and Arrays
		unsigned int VA_Main;
		unsigned int VBO_Vertex;
		unsigned int VBO_Faces;

		GeometricBoundingBox *m_Box;
		void loadToGPU();
		ModelGL *m_Model;

};


#endif
