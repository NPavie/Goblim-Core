/**
 * @author     G.Gilet, Guillaume.Gilet@unilim.fr
 * @date       October 8, 2012
 * @version    1.0 (alpha)
 */

#ifndef __CAMERA__
#define __CAMERA__

#include "Engine/Base/Frame.h"
#include <math.h>
#include <string>

/**
 * @brief      Class to manage a camera in your virtual scene
 *
 * @details    Verbose description of class details.
 *
 */


class Camera
{
	public:
		/**
		 * @brief      Creates a new Camera.
		 */
		Camera(std::string name= "");

		/**
		 * @brief      Destroy a Camera
		 */
		~Camera();

		/**
		 * @brief      Link the camera to the frame f (usually the scene frame).
		 * @param      f  the target frame
		 */
		void link(Frame *f);

		//*********************** Camera position and orientation transformation**********************//

		/**
		 * @brief Define the position frame, using standard glut lookAt syntax.
		 * @details Place the camera frame in the virtual scene : Point the Z-axis of camera frame toward point pointTo, using center as camera frame position and up as Y vector (pointed toward the top of the screen)
		 *
		 * @param pointTo the 3D point faced by the camera
		 * @param center the center of the camera in 3D space
		 * @param up the up direction (the top of the screen) in 3D space.
		 */
		void lookAt(glm::vec3 pointTo,glm::vec3 center,glm::vec3 up);


		/**
		 * @brief Rotate the camera frame of angle degrees around the axis v .
		 *
		 * @param v the axis of rotation
		 * @param angle the angle of the rotation (in degrees)
		 */
		void rotate(glm::vec3 v,float angle);

		/**
		 * @brief Translate the camera frame by vector v.
		 *
		 * @param v the vector of translation
		 */
		void translate(glm::vec3 v);

		//*********************** Camera projection transformation **********************//
		/**
		 * @brief Set the projection matrix of camera
		 *
		 * @param m the projection matrix
		 */

		void setProjectionMatrix(glm::mat4 & m);

		//*********************** Define a perspective projection *********************//

		/**
		 * @brief Set Perspective projection matrix using horizontal half field of view, pixel aspect ratio (width/height ratio of one pixel),  distance to near and far clipping planes.
		 *
		 * @param foV to define half Field of View (in degrees) .
		 * @param aspectRatio as aspect ratio.
		 * @param front as distance of near clipping plane.
		 * @param back as distance of back clipping plane.
		 */
		void setPerspectiveProjection(float foV,float aspectRatio,float front,float back );


		/**
		 * @brief Set Perspective Frustum.<a href="http://en.wikipedia.org/wiki/Viewing_frustum.">Wiki link</a>
		 *
		 * @param left as frustum left plane.
		 * @param right as frustum right plane.
		 * @param bottom as frustum bottom plane.
		 * @param top as frustum top plane.
		 * @param near as near clipping plane.
		 * @param far as far clipping plane.
		 */
		void setFrustum(float left,float right,	float bottom,float top,	float near,float far) ;

		//*********************** Define orthographic projection ********************//
		/**
		 *@brief Set orthographic projection.
		 *
		 * @param left as frustum left plane.
		 * @param right as frustum right plane.
		 * @param bottom as frustum bottom plane.
		 * @param top as frustum top plane.
		 * @param near as near clipping plane.
		 * @param far as far clipping plane.
		 */
		void setOrthographicProjection(float left,float right,float bottom,float top,float near,float far);



		//******************** Frame conversion operators **************************//
		/**
		 * @brief Convert coordinates of a 3D point pt expressed in Camera Frame to coordinates expressed in frame fDest.
		 *
		 * @param pt for point coordinates (expressed in Camera frame).
		 * @param fDest as destination frame.
		 * @return coordinates of point pt in the frame f.
		 * @warning frame fDest and camera frame must be linked in the scene graph (there must be a path of conversion between f and camera frame) for success of conversion operation. Results of conversion between unlinked frames are undefined
		 **/
		glm::vec3 convertPtTo(glm::vec3 pt,Frame *fDest);

		/**
		 * @brief Convert coordinates of a 3D direction dir expressed in Camera Frame to coordinates expressed in frame fDest.
		 *
		 * @param dir for direction coordinates (expressed in Camera frame).
		 * @param fDest as destination frame.
		 * @return coordinates of direction dir in the frame f.
		 * @warning frame fDest and camera frame must be linked in the scene graph (there must be a path of conversion between fDest and camera frame) for success of conversion operation. Results of conversion between unlinked frames are undefined
		 **/
		glm::vec3 convertDirTo(glm::vec3 dir,Frame *fDest);

		/**
		 * @brief Convert coordinates of a 3D point pt expressed in frame fSource to coordinates expressed in camera frame.
		 *
		 * @param pt for point coordinates (expressed in fSource frame).
		 * @param fSource as source frame.
		 * @return coordinates of point pt in the camera frame.
		 * @warning frame fSource and camera frame must be linked in the scene graph (there must be a path of conversion between f and camera frame) for success of conversion operation. Results of conversion between unlinked frames are undefined
		 **/

		glm::vec3 convertPtFrom(glm::vec3 pt,Frame *fSource);

		/**
		 * @brief Convert coordinates of a 3D direction dir expressed in frame fSource to coordinates expressed in camera frame.
		 *
		 * @param dir for direction coordinates (expressed in fSource frame).
		 * @param fSource as source frame.
		 * @return coordinates of direction dir in the camera frame.
		 * @warning frame fSource and camera frame must be linked in the scene graph (there must be a path of conversion between f and camera frame) for success of conversion operation. Results of conversion between unlinked frames are undefined
		 **/
		glm::vec3 convertDirFrom(glm::vec3 & d,Frame*f);

		// *********************** Members accessors ***********************//

		/**
		 * @brief Get distance of near clipping plane from camera frame center
		 * @return distance of near clipping plane
		 **/
		float getZnear();

		/**
		 * @brief Get distance of far clipping plane from camera frame center
		 * @return distance of far clipping plane
		 **/
		float getZfar();

		/**
		 * @brief Set distance of near clipping plane from camera frame center
		 * @param distance of near clipping plane
		 * @note Recompute projection matrix
		 **/
		void setZnear(float n);

		/**
		 * @brief Set distance of far clipping plane from camera frame center
		 * @param distance of far clipping plane
		 * @note Recompute projection matrix
		 **/
		void setZfar(float f);

		/**
		 * @brief Return field of view angle (in degrees) of camera.
		 * @return field of view angle
		 **/
		float getFoV();

		/**
		 * @brief Set field of view angle (in degrees).
		 * @param half field of view angle
		 * @note Recompute projection matrix
		 **/
		void setFoV(float v);

		/**
		 * @brief Return pixel aspect ratio
		 * @return pixel aspect ratio
		 **/
		float getAspectRatio();

		/**
		 * @brief Set pixel aspect ratio
		 * @param pixel aspect ratio
		 * @note Recompute projection matrix
		 **/
		void setAspectRatio(float a);


		/**
		 * @brief Return camera frame
		 * @return camera frame
		 **/
		Frame *getFrame();

		
		/**
		 * @brief Return projection Matrix
		 * @return projection Matrix
		 **/
		glm::mat4 getProjectionMatrix();

		/**
		 * @brief Return modelview matrix (transformation from frame f to camera space)
		 * @return modelview Matrix
		 **/
		glm::mat4 getModelViewMatrix(Frame *f);

		/**
		 * @brief Set up camera position and orientation from matrix m
		 **/

		void setUpFromMatrix(glm::mat4 & m);

			/**
		 * @brief Return true if camera moved since last update
		 * @return true if camera moved since last update
		 **/
		bool needUpdate();
		/**
		* @brief Flag camera frame as having changed since last update
		* @param r for flagging frame as needing update
		 **/
		virtual void setUpdate(bool r);

		/**
		* @brief return camera name
		 **/
		const std::string getName();
	protected:

		Frame *frame;		///< Camera position frame
		Frame *projection_frame; ///< Camera projection frame

		float aspectRatio;	///< pixel aspect ratio (width/height ratio of one pixel)

		float foV;	///< field of view of the camera

		float Znear;	///< distance from camera frame center to near clipping plane
		float Zfar;		///< distance from camera frame center to far clipping plane

		//!  Disable Projection.
		void disableProjection(){frame->attachTo(NULL);};
		//!  Enable Projection.
		void enableProjection(){frame->attachTo(projection_frame);};

			std::string m_Name;
};



#endif
