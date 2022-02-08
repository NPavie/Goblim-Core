#pragma once
#include <string>
#include <stdlib.h>
#include <glm/glm.hpp>

class Resource
{
public:
	Resource(std::string name);
	~Resource();

	std::string name();
	virtual void display(const glm::vec4 & box = glm::vec4(0.0,0.0,1.0,1.0));

protected:
	std::string m_Name;
	virtual bool load();

};
	  