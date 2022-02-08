#ifndef __ROTATION_ANIMATOR_H__
#define __ROTATION_ANIMATOR_H__

#include "Engine/Base/Animator.h"

class RotationAnimator : public Animator
{
protected:
	double m_rotationTime; // in millisecond

	glm::vec3 axis;
public  :
	RotationAnimator(Frame* f, double t, glm::vec3 a=glm::vec3(0,1,0));
	~RotationAnimator();

	void setRotationTime(const double t);
	double getRotationTime() const;

	void setAxis(const glm::vec3 a);
	glm::vec3 getAxis() const;

	void animate(const double elapsedTime, Frame* f);
};

#endif // __ROTATION_ANIMATOR_H__

