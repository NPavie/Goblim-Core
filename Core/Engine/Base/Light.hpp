#ifndef _GOBLIM_LIGHT_
#define _GOBLIM_LIGHT_
#include <glm/glm.hpp>
#include <string.h>
class Light
{
public:
	Light(std::string name,glm::vec3 color = glm::vec3(1.0f));

private:
	glm::vec4 m_Color;
	std::string m_Name;
}

#endif