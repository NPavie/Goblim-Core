#include "Effect.h"

Effect::Effect()
{
	valid = false;
}
Effect::Effect(std::string name,std::string className)
{
	this->m_Name = name;
	this->m_ClassName = className;
}
Effect::~Effect()
{
}
void Effect::apply()
{
}
const std::string Effect::getName()
{
	return m_Name;
}
