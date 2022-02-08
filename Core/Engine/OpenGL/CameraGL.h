/**
 * @author     G.Gilet, Guillaume.Gilet@unilim.fr
 * @date       October 8, 2012
 * @version    1.0 (alpha)
 */

#ifndef __CAMERA_GL__
#define __CAMERA_GL__

#include "Engine/Base/Camera.h"
#include "GPUResources/Buffers/GPUBuffer.h"

#define MATRICES_SSBO_BINDINGS 1

/**
 * @brief      Class to manage a OpenGL camera in your virtual scene
 *
 * @details    Verbose description of class details.
 *
 */

// cas étrange : si on rajoute un vec4 dans le buffer et qu'on utilise ce vec4, le SSBO fonctionne
struct ProjectionMatrices
{
	glm::mat4 Proj;
	glm::mat4 View;
	glm::mat4 ViewProj;
	glm::mat4 ViewProjInv;
	glm::mat4 ViewProjNormal;
};


class CameraGL : public Camera
{
	public:
		/**
		 * @brief      Creates a new Camera.
		 */
		CameraGL(std::string name= "");

		/**
		 * @brief      Destroy a Camera
		 */
		~CameraGL();

		virtual void setUpdate(bool r);

		virtual void bind();
		virtual void updateBuffer();
		
		
	private:
		GPUBuffer *m_ProjBuffer;

		ProjectionMatrices param;
		

};



#endif
