#pragma once

#include "glm/glm.hpp"
#include <string>
#include <sstream>
#include <vector>

#include "Spot.h"


class GaussianSplat {
public:
    /** @brief 
        @param
        @param
        @param
        @param
        @param
     */
	GaussianSplat(glm::vec2 position, float magnitude = 1.0f, glm::vec2 size = glm::vec2(1.0), float rotation = 0.0f, glm::vec4 & color = glm::vec4(1.0f));
	

    /** @brief
        @param
     */
    glm::vec4 operator() (glm::vec2 position);
	
    /** @brief
     */
	float spectrum(glm::vec2 frequency);

    /** @brief
     */
	std::string to_string();

	glm::mat3 GaussianMatrix;
	glm::vec4 color;
	glm::vec2 size;
	glm::vec2 position;
	float magnitude;
	float rotation;


    static std::vector<GaussianSplat*> readListFromFile(const char* filename);

	static std::vector<GaussianSplat*> readListFromSVGExtraction(const char* filename);


};