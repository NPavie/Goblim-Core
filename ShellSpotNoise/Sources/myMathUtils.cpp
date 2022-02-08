
#include "myMathUtils.hpp"

namespace MathUtils
{

	float clamp(float x, float min, float max)
	{
		if (x < min) x = min;
		if (x > max) x = max;
		return x;
	}

	int clamp(int x, int min, int max)
	{
		if (x < min) x = min;
		if (x > max) x = max;
		return x;
	}

	// dot2 = norme au carré d'un vecteur
	float dot2(glm::vec3 v) { return glm::dot(v, v); }
	float dot2(glm::vec2 v) { return glm::dot(v, v); }

	glm::vec3 clamp(glm::vec3 testedVec, glm::vec3 min_v, glm::vec3 max_v)
	{
		for (int i = 0; i < 3; i++)
		{
			if (testedVec[i] < min_v[i]) testedVec[i] = min_v[i];
			if (testedVec[i] > max_v[i]) testedVec[i] = max_v[i];
		}
		return testedVec;
	}

	// signed distance to a 2D triangle - IQ
	float sdTriangle(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p)
	{
		// edges
		glm::vec2 e0 = p1 - p0;
		glm::vec2 e1 = p2 - p1;
		glm::vec2 e2 = p0 - p2;

		// Distance of point to the
		glm::vec2 v0 = p - p0;
		glm::vec2 v1 = p - p1;
		glm::vec2 v2 = p - p2;

		glm::vec2 pq0 = v0 - e0* clamp(glm::dot(v0, e0) / glm::dot(e0, e0), 0.0, 1.0);
		glm::vec2 pq1 = v1 - e1* clamp(glm::dot(v1, e1) / glm::dot(e1, e1), 0.0, 1.0);
		glm::vec2 pq2 = v2 - e2* clamp(glm::dot(v2, e2) / glm::dot(e2, e2), 0.0, 1.0);

		// cross products
		glm::vec2 d = glm::min(glm::min(glm::vec2(glm::dot(pq0, pq0), v0.x*e0.y - v0.y*e0.x),
			glm::vec2(glm::dot(pq1, pq1), v1.x*e1.y - v1.y*e1.x)),
			glm::vec2(glm::dot(pq2, pq2), v2.x*e2.y - v2.y*e2.x));

		return -sqrt(d.x)*glm::sign(d.y);
	}



	float udTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 p)
	{
		glm::vec3 v21 = v2 - v1; glm::vec3 p1 = p - v1;
		glm::vec3 v32 = v3 - v2; glm::vec3 p2 = p - v2;
		glm::vec3 v13 = v1 - v3; glm::vec3 p3 = p - v3;
		glm::vec3 nor = glm::cross(v21, v13);

		// distance non signé
		return sqrt((glm::sign(glm::dot(glm::cross(v21, nor), p1)) +
			glm::sign(glm::dot(glm::cross(v32, nor), p2)) +
			glm::sign(glm::dot(glm::cross(v13, nor), p3))<2.0)
			?
			glm::min(glm::min(
			dot2(v21*clamp(glm::dot(v21, p1) / dot2(v21), 0.0, 1.0) - p1),
			dot2(v32*clamp(glm::dot(v32, p2) / dot2(v32), 0.0, 1.0) - p2)),
			dot2(v13*clamp(glm::dot(v13, p3) / dot2(v13), 0.0, 1.0) - p3))
			:
			glm::dot(nor, p1)*glm::dot(nor, p1) / dot2(nor));
	}


	// code form real-time collision detection - ça marche !
	glm::vec3 triangleClosestPoint(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 & barycentriqueCoords)
	{
		// Check if P in vertex region outside A
		glm::vec3 ab = b - a;
		glm::vec3 ac = c - a;
		glm::vec3 ap = p - a;
		float d1 = glm::dot(ab, ap);
		float d2 = glm::dot(ac, ap);
		if (d1 <= 0.0f && d2 <= 0.0f)
		{
			barycentriqueCoords = glm::vec3(1, 0, 0);
			return a; // barycentric coordinates (1,0,0)
		}

		// Check if P in vertex region outside B
		glm::vec3 bp = p - b;
		float d3 = glm::dot(ab, bp);
		float d4 = glm::dot(ac, bp);
		if (d3 >= 0.0f && d4 <= d3)
		{
			barycentriqueCoords = glm::vec3(0, 1, 0);
			return b; // barycentric coordinates (0,1,0)
		}

		// Check if P in edge region of AB, if so return projection of P onto AB
		float vc = d1*d4 - d3*d2;
		if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
		{
			float v = d1 / (d1 - d3);
			barycentriqueCoords = glm::vec3(1.0 - v, v, 0);
			return a + v * ab; // barycentric coordinates (1-v,v,0)
		}
		// Check if P in vertex region outside C
		glm::vec3 cp = p - c;
		float d5 = glm::dot(ab, cp);
		float d6 = glm::dot(ac, cp);
		if (d6 >= 0.0f && d5 <= d6)
		{
			barycentriqueCoords = glm::vec3(0, 0, 1);
			return c; // barycentric coordinates (0,0,1)
		}

		// Check if P in edge region of AC, if so return projection of P onto AC
		float vb = d5*d2 - d1*d6;
		if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
		{
			float w = d2 / (d2 - d6);
			barycentriqueCoords = glm::vec3(1.0 - w, 0, w);
			return a + w * ac; // barycentric coordinates (1-w,0,w)
		}
		// Check if P in edge region of BC, if so return projection of P onto BC
		float va = d3*d6 - d5*d4;
		if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
		{
			float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
			barycentriqueCoords = glm::vec3(0, 1.0 - w, w);
			return b + w * (c - b); // barycentric coordinates (0,1-w,w)
		}
		// P inside face region. Compute Q through its barycentric coordinates (u,v,w)
		float denom = 1.0f / (va + vb + vc);
		float v = vb * denom;
		float w = vc * denom;
		float u = 1.0f - v - w;
		barycentriqueCoords = glm::vec3(u, v, w);
		return a + ab * v + ac * w; // = u*a + v*b + w*c, u = va * denom = 1.0f - v - w


	}
};
