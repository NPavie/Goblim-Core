#include "Engine/OpenGL/ModelGL.h"

#include "Engine/OpenGL/BoundingBox/BoundingBoxModelGL.h"

ModelGL::ModelGL(string name,bool loadnow):
	Model(name,loadnow)
{
	if (loadnow)
		loadToGPU();
		
	
	
}

ModelGL::~ModelGL()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO_Vertex);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO_Normals);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO_Faces);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO_TexCoords);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO_Tangents);
	
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO_BorderData);

	
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VA_Main);


}

void ModelGL::loadToGPU()
{
	if (m_Model != NULL)
	{

		if (m_Model->listVertex.size() > 0)
		{
			glGenBuffersARB(1, &VBO_Vertex);
			glBindBufferARB(GL_ARRAY_BUFFER, VBO_Vertex);
			glBufferDataARB(GL_ARRAY_BUFFER,  m_Model->nb_vertex*sizeof(glm::vec3), &(m_Model->listVertex.front()), GL_STATIC_DRAW_ARB);
		}
		if (m_Model->listFaces.size() > 0)
		{
			glGenBuffersARB(1, &VBO_Faces);
			glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, VBO_Faces);
			glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, m_Model->nb_faces*sizeof(Face), &(m_Model->listFaces.front()), GL_STATIC_DRAW_ARB);
		}

		// Create VBO and Send data to GPU
		if (m_Model->listNormals.size() > 0)
		{
			glGenBuffersARB(1, &VBO_Normals);
			glBindBufferARB(GL_ARRAY_BUFFER, VBO_Normals);
			glBufferDataARB(GL_ARRAY_BUFFER,  m_Model->nb_vertex*sizeof(glm::vec3), &(m_Model->listNormals.front()), GL_STATIC_DRAW_ARB);
		}
		if (m_Model->listCoords.size() != 0)
		{

			glGenBuffersARB(1, &VBO_TexCoords);
			glBindBufferARB(GL_ARRAY_BUFFER, VBO_TexCoords);
			glBufferDataARB(GL_ARRAY_BUFFER,  m_Model->nb_vertex*sizeof(glm::vec3), &(m_Model->listCoords.front()), GL_STATIC_DRAW_ARB);
		}

		if (m_Model->listTangents.size() > 0)
		{
			glGenBuffersARB(1, &VBO_Tangents);
			glBindBufferARB(GL_ARRAY_BUFFER, VBO_Tangents);
			glBufferDataARB(GL_ARRAY_BUFFER,  m_Model->nb_vertex*sizeof(glm::vec4), &(m_Model->listTangents.front()), GL_STATIC_DRAW_ARB);
		}

		// Addon : Sending border information to the GPU
		if (m_Model->listBorderNeighbours.size() > 0)
		{
			glGenBuffersARB(1, &VBO_BorderData);
			glBindBufferARB(GL_ARRAY_BUFFER, VBO_BorderData);
			glBufferDataARB(GL_ARRAY_BUFFER,  m_Model->nb_vertex*sizeof(glm::vec3), &(m_Model->listBorderNeighbours.front()), GL_STATIC_DRAW_ARB);
		}


		glGenVertexArrays(1, &VA_Main);
		glBindVertexArray(VA_Main);

		glBindBuffer(GL_ARRAY_BUFFER,VBO_Vertex);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
		glEnableVertexAttribArray(0);	//POS

		if (m_Model->listBorderNeighbours.size() != 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER,VBO_BorderData);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
			glEnableVertexAttribArray(1);	//isBorder
		}

		if (m_Model->listNormals.size() != 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, VBO_Normals);
			glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
			glEnableVertexAttribArray(2);	//Normal
		}

		if (m_Model->listCoords.size() != 0)
		{

			glBindBuffer(GL_ARRAY_BUFFER, VBO_TexCoords);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
			glEnableVertexAttribArray(3); //TCoord

			glBindBuffer(GL_ARRAY_BUFFER, VBO_Tangents);
			glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), 0);
			glEnableVertexAttribArray(4);	//Tangents
		}


		glBindVertexArray(0);
		glBindBufferARB(GL_ARRAY_BUFFER,0);
		bboxGL = new BoundingBoxModelGL(this->getGeometricModel()->box);
	}
	
}






void ModelGL::drawGeometry(GLint type)
{
	if (m_Model != NULL)
	{
		glBindVertexArray(VA_Main);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, VBO_Faces);

		glDrawRangeElements(type,0,3*m_Model->nb_faces,3*m_Model->nb_faces, GL_UNSIGNED_INT,0);

		glBindVertexArray(0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
	}

}

void ModelGL::drawInstancedGeometry(GLint type, GLint nbInstance)
{
	if (m_Model != NULL)
	{
		glBindVertexArray(VA_Main);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, VBO_Faces);
		glDrawElementsInstanced(type, 3*m_Model->nb_faces, GL_UNSIGNED_INT, 0, nbInstance);
		glBindVertexArray(0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
	}
}
