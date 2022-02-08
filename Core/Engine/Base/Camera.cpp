/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */
#include "Engine/Base/Camera.h"
#include <glm/gtc/matrix_transform.hpp> 


Camera::Camera(std::string name)
{
	Znear = 0.01f;
	Zfar = 10000.0f;
	aspectRatio = 1.0f;
	foV = 45.0;
	frame = new Frame();
	frame->setAsCameraFrame(true);
	projection_frame = new Frame();
	projection_frame->setAsCameraFrame(true);
	frame->attachTo(projection_frame);
	m_Name = name;
}
const std::string Camera::getName()
{
	return m_Name;
}
Camera::~Camera()
{
	if (frame != NULL)	delete frame;
	if (projection_frame != NULL )	delete projection_frame;
}

void Camera::link(Frame *f)
{
	f->attachTo(frame);
	setUpdate(true);

}


void Camera::setOrthographicProjection(float left,float right,float bottom,float top,float near,float far)
{
	this->Znear = near;
	this->Zfar = far;

	glm::mat4 pmat = glm::ortho(left,right,bottom,top,near,far);
	projection_frame->setUpFromMatrix(pmat);
	setUpdate(true);

}

void Camera::setFrustum(float left,float right,	float bottom,float top,	float near,float far)
{
	this->Znear = near;
	this->Zfar = far;

	glm::mat4 pmat = glm::frustum(left,right,bottom,top,near,far);
	projection_frame->setUpFromMatrix(pmat);
	setUpdate(true);

}


void Camera::setPerspectiveProjection(float foV,float aspectRatio,float near,float far )
{
	this->foV = foV;
	this->aspectRatio = aspectRatio;
	this->Znear = near;
	this->Zfar = far;

	glm::mat4 projmat = glm::perspective(foV, aspectRatio, near, far);

	projection_frame->setUpFromMatrix(projmat);
	setUpdate(true);
}
void Camera::lookAt(glm::vec3 pointTo,glm::vec3 center,glm::vec3 up)
{
	frame->setUpFromMatrix(glm::lookAt(center,pointTo,up));	// WArning pb doc GLM
	setUpdate(true);
}

void Camera::rotate(glm::vec3 v,float angle)
{

	//// Save actual frame matrix
	glm::mat4 actual = *(frame->getMatrix());

	// load identity in frame
	frame->loadIdentity();
	// load create a rotation matrix in frame
	frame->rotate(v,angle);

	glm::mat4 mrot= glm::inverse(*(frame->getMatrix()));

	//// PreMultiply old matrix with rotation matrix
	frame->setUpFromMatrix( mrot * actual);
	setUpdate(true);

}
void Camera::translate(glm::vec3 v)
{
	glm::mat4 actual = *(frame->getMatrix());

	// load identity in frame
	frame->loadIdentity();
	// load create a rotation matrix in frame
	frame->translate(v);

	glm::mat4 mtrans= glm::inverse(*(frame->getMatrix()));


	// PreMultiply old matrix with translation matrix
	frame->setUpFromMatrix( mtrans * actual);

	glm::mat4 m = *(frame->getMatrix());
	setUpdate(true);

}

glm::vec3 Camera::convertPtTo(glm::vec3 p,Frame *f)
{
	frame->attachTo(NULL);
	glm::mat4 m = glm::inverse(f->getTransformMatrix());
	frame->attachTo(projection_frame);
	glm::vec4 localpt = (m * glm::vec4(p,1.0));

	return(glm::vec3(localpt));
}
glm::vec3 Camera::convertDirTo(glm::vec3 d,Frame *f)
{
	frame->attachTo(NULL);
	glm::mat4 m = glm::inverse(f->getTransformMatrix());
	frame->attachTo(projection_frame);
	glm::vec4 localdir = (m * glm::vec4(d,0.0));

	return(glm::vec3(localdir));
}
glm::vec3 Camera::convertPtFrom(glm::vec3 p,Frame*f)
{
	frame->attachTo(NULL);
	glm::mat4 m = f->getTransformMatrix();
	frame->attachTo(projection_frame);
	glm::vec4 localpt = (m * glm::vec4(p,1.0));

	return(glm::vec3(localpt));
}
glm::vec3 Camera::convertDirFrom(glm::vec3 & d,Frame*f)
{
	frame->attachTo(NULL);
	glm::mat4 m = f->getTransformMatrix();
	frame->attachTo(projection_frame);
	glm::vec4 localdir = (m * glm::vec4(d,0.0));
	return(glm::vec3(localdir));
}


void Camera::setProjectionMatrix(glm::mat4 & m)
{
	projection_frame->setUpFromMatrix(m);

}
void Camera::setUpFromMatrix(glm::mat4 & m)
{
	frame->setUpFromMatrix(m);
}


float Camera::getZnear()
{
	return(Znear);
}
void Camera::setZnear(float n)
{
	this->Znear = n;
	setPerspectiveProjection(foV,aspectRatio,Znear,Zfar);
}
float Camera::getZfar()
{
	return(Zfar);
}
void Camera::setZfar(float f)
{
	this->Zfar = f;
	setPerspectiveProjection(foV,aspectRatio,Znear,Zfar);
}
float Camera::getFoV()
{
	return(foV);
}
void Camera::setFoV(float v)
{
	this->foV = v;
	setPerspectiveProjection(foV,aspectRatio,Znear,Zfar);
}
float Camera::getAspectRatio()
{
	return(aspectRatio);
}
void Camera::setAspectRatio(float a)
{
	this->aspectRatio = a;
	setPerspectiveProjection(foV,aspectRatio,Znear,Zfar);
}

Frame *Camera::getFrame()
{
	return(frame);
}

glm::mat4 Camera::getProjectionMatrix()
{
	return *(projection_frame->getMatrix());
}
glm::mat4 Camera::getModelViewMatrix(Frame *f)
{
	glm::mat4 MV;
	this->disableProjection();
	MV = f->getTransformMatrix();
	this->enableProjection();

	return MV;
}
bool Camera::needUpdate()
{
	return(frame->updateNeeded() ||projection_frame->updateNeeded());
}

void Camera::setUpdate(bool r)
{
	frame->setUpdate(r);
	projection_frame->setUpdate(r);
}


