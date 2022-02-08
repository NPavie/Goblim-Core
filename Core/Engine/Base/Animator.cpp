#include "Animator.h"

Animator::Animator(Frame* f)
{
	m_reference = f;
}

Animator::~Animator()
{
}

Frame* Animator::reference()
{
	return m_reference;
}

void Animator::setReference(Frame* f)
{
	m_reference = f;
}

void Animator::draw()
{
}
