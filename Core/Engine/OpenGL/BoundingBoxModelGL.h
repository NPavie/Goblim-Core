/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */
#ifndef _BOUNDINGBOXMODELGL_H
#define _BOUNDINGBOXMODELGL_H


#include <glad/glad.h>
#include "Engine/Base/BoundingBox/GeometricBoundingBox.h"

class ModelGL;


class BoundingBoxModelGL 
{
	public:
        /*! \brief Constructor of an openGL bounding box
		 *	\param1 box GeometricBoundingBox object
         */
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
