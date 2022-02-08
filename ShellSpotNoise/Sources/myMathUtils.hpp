
#pragma once

#include <glm/glm.hpp>

namespace MathUtils
{
	
	
	// norme au carré d'un vec3
	float dot2(glm::vec3 v);
	// norme au carré d'un vec2
	float dot2(glm::vec2 v);

	// Clamping d'un vec3
	glm::vec3 clamp(glm::vec3 testedVec, glm::vec3 min_v, glm::vec3 max_v);

	float clamp(float x, float min, float max);
	
	int clamp(int x, int min, int max);

	// Distance signé entre un point et un triangle (2D)
	float sdTriangle(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p);

	// Distance non signé entre un point et un triangle (3D)
	float udTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 p);

	// Point le plus proche sur un triangle par rapport à un point p avec coordonnées barycentriques
	glm::vec3 triangleClosestPoint(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 & outBarycentriqueCoords);

};
