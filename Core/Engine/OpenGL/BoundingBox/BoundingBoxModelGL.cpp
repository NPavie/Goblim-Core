#include "BoundingBoxModelGL.h"
#include "Engine/OpenGL/ModelGL.h"


BoundingBoxModelGL::BoundingBoxModelGL(GeometricBoundingBox *box)
{
	m_Box = box;
	loadToGPU();
}

BoundingBoxModelGL::~BoundingBoxModelGL()
{
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO_Vertex);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &VBO_Faces);
	
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VA_Main);


}

void BoundingBoxModelGL::loadToGPU()
{
	if (m_Box != NULL)
	{
		std::vector<glm::vec3> vertices = m_Box->getVertices();

		if (vertices.size() > 0)
		{
			glGenBuffersARB(1, &VBO_Vertex);
			glBindBufferARB(GL_ARRAY_BUFFER, VBO_Vertex);
			glBufferDataARB(GL_ARRAY_BUFFER,  vertices.size()*sizeof(glm::vec3), &(vertices.front()), GL_STATIC_DRAW_ARB);
		
		std::vector <glm::uvec4> faces;
		faces.push_back(glm::vec4(0,1,2,3));
		faces.push_back(glm::vec4(4,5,6,7));
		faces.push_back(glm::vec4(0,6,5,3));

		faces.push_back(glm::vec4(1,7,4,2));
		faces.push_back(glm::vec4(2,3,5,4));
		faces.push_back(glm::vec4(1,0,6,7));


		
		glGenBuffersARB(1, &VBO_Faces);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, VBO_Faces);
		glBufferDataARB(GL_ELEMENT_ARRAY_BUFFER_ARB, 6*sizeof(glm::uvec4), &(faces.front()), GL_STATIC_DRAW_ARB);
	

	
		glGenVertexArrays(1, &VA_Main);
		glBindVertexArray(VA_Main);

		glBindBuffer(GL_ARRAY_BUFFER,VBO_Vertex);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), 0);
		glEnableVertexAttribArray(0);	//POS

		glBindVertexArray(0);
		glBindBufferARB(GL_ARRAY_BUFFER,0);
		}
	}
}



void BoundingBoxModelGL::drawGeometry()
{
	if (m_Box != NULL)
	{
		glBindVertexArray(VA_Main);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, VBO_Faces);

		glDrawRangeElements(GL_QUADS,0,24,24, GL_UNSIGNED_INT,0);

		glBindVertexArray(0);
		glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB,0);
	}

}

