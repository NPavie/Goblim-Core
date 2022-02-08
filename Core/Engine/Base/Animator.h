#ifndef __ANIMATOR_H__
#define __ANIMATOR_H__

#include "Engine/Base/Frame.h"

class Animator
{
protected :
	Frame* m_reference;
public  :
	Animator(Frame* f=NULL);
	~Animator();

	Frame* reference();
	void setReference(Frame* f);

	virtual void animate(const double elapsedTime, Frame* f) = 0;
	virtual void draw();
};

#endif // __ANIMATOR_H__

