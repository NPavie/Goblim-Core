#include "RotationAnimator.h"
#include <cstdio>
RotationAnimator::RotationAnimator(Frame* f, double t, glm::vec3 a) : Animator(f), m_rotationTime(t), axis(a)
{
}

RotationAnimator::~RotationAnimator()
{
}

void RotationAnimator::setRotationTime(const double t)
{
	m_rotationTime = t;
}

double RotationAnimator::getRotationTime() const
{
	return m_rotationTime;
}

void RotationAnimator::setAxis(glm::vec3 a)
{
	axis = a;
}

glm::vec3 RotationAnimator::getAxis() const
{
	return axis;
}

void RotationAnimator::animate(const double elapsedTime, Frame* f)
{
	float angle = 360.0*elapsedTime/m_rotationTime;
	f->rotate(f->convertDirTo(axis, m_reference), angle);
}
