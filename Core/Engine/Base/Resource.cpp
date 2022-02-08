#include "Engine/Base/Resource.h"

Resource::Resource(std::string name)
{
	m_Name = name;
}

Resource::~Resource()
{
}

bool Resource::load()
{

	return true;
}

std::string Resource::name()
{
	return m_Name;
}

void Resource::display(const glm::vec4 & box)
{

}