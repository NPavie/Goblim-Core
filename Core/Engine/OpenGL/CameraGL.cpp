/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */
#include "CameraGL.h"
#include "Engine/Base/Scene.h"

void displayMat(glm::mat4 & m)
{
	std::cout << m[0][0] << " " << m[0][1] << " " << m[0][2] << " " << m[0][3] << " " << std::endl;
	std::cout << m[1][0] << " " << m[1][1] << " " << m[1][2] << " " << m[1][3] << " " << std::endl;
	std::cout << m[2][0] << " " << m[2][1] << " " << m[2][2] << " " << m[2][3] << " " << std::endl;
	std::cout << m[3][0] << " " << m[3][1] << " " << m[3][2] << " " << m[3][3] << " " << std::endl;
}

CameraGL::CameraGL(std::string name):
	Camera(name)
{
	m_ProjBuffer = new GPUBuffer("Common-Projection");
	m_ProjBuffer->create(sizeof(ProjectionMatrices) , GL_SHADER_STORAGE_BUFFER  ,GL_DYNAMIC_COPY);
	m_ProjBuffer->setBindingPoint(COMMON_SSBO_BINDINGS);
		
}
CameraGL::~CameraGL()
{
	//delete m_ProjBuffer;
}



void CameraGL::updateBuffer()
{
	param.Proj = getProjectionMatrix();
	param.View = *(frame->getMatrix());
	param.ViewProj = param.Proj * param.View;
	param.ViewProjInv = glm::inverse(param.ViewProj);
	param.ViewProjNormal = glm::transpose(glm::inverse(param.ViewProj));
	
	//displayMat(param.ViewProj);
	//displayMat(param.ViewProjInv);
	//std::cout << param.View[3].x << ";" << param.View[3].y << ";" << param.View[3].z << ";" << param.View[3].w << std::endl;

	// Il n'y a que le memcpy sur la totalité du buffer qui semble fonctionner correctement pour que la mémoire soit bien copier
	m_ProjBuffer->bind();
	ProjectionMatrices* ptr = (ProjectionMatrices* ) m_ProjBuffer->map(GL_WRITE_ONLY);
	if (ptr != NULL)
	{
		// Recommandation : il semblerait qu'une petite erreur se glisse dans le transfert mémoire si on utilise pas memcpy
		memcpy(ptr, &param, sizeof(param));
		m_ProjBuffer->unMap();
	} //else std::cout << "Error mapping Projections matrices to buffer \n";
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); // A mon avis, si le buffer n'est pas release, le map n'est pas valider par l'implémentation AMD
	
	m_ProjBuffer->bind();


	//// Affichage pour le debuggage
	//std::cout << " ViewProj : " << std::endl;
	//displayMat(param.ViewProj);


	
}

void CameraGL::setUpdate(bool r)
{
	Camera::setUpdate(r);
	if (r)
	{
		updateBuffer();
	}

}

void CameraGL::bind()
{
	m_ProjBuffer->bind();
}