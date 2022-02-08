#include "Material.h"

Material::Material()
{
	valid = false;
}
Material::Material(std::string name,std::string className)
{
	this->m_Name = name;
	this->m_ClassName = className;
}
Material::~Material()
{
}
void Material::render(Node *o)
{
}
const std::string Material::getName()
{
	return m_Name;
}
void Material::update(Node *o,const int elapsedTime)
{
}