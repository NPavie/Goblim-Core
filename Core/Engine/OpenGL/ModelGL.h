/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */
#ifndef _MODELGL_H
#define _MODELGL_H


#include <GL/glew.h>
#include "Engine/Base/Model.h"

#include "Engine/OpenGL/BoundingBox/BoundingBoxModelGL.h"

class ModelGL : public Model
{
	public:
		ModelGL(std::string name,bool loadnow = true);
		~ModelGL();
		virtual void drawGeometry(GLint type=GL_TRIANGLES);
		virtual void drawInstancedGeometry(GLint type=GL_TRIANGLES, GLint nbInstance = 50);
		void loadToGPU();

		BoundingBoxModelGL *bboxGL;


	protected:
		// Buffers and Arrays
		unsigned int VA_Main;
		unsigned int VBO_Vertex;
		unsigned int VBO_Faces;
		unsigned int VBO_TexCoords;
		unsigned int VBO_Normals;
		unsigned int VBO_Tangents;
		unsigned int VBO_BorderData;

};


#endif
