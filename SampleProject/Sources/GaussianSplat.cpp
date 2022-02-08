#include "GaussianSplat.h"

#include <fstream>

GaussianSplat::GaussianSplat(glm::vec2 position, float magnitude, glm::vec2 size, float rotation, glm::vec4 & color)
	:color(color),magnitude(magnitude), size(size),rotation(rotation),position(position)
{
	glm::mat3 scaleMatrix = glm::mat3(1.0f);
	scaleMatrix[0].x = size.x;
	scaleMatrix[1].y = size.y;

	glm::mat3 shiftMatrix = glm::mat3(1.0f);
	shiftMatrix[2].x = position.x;
	shiftMatrix[2].y = position.y;

	glm::mat3 rotationMatrix = glm::mat3(1.0f);
	rotationMatrix[0].x = cos(rotation);
	rotationMatrix[0].y = sin(rotation);

	rotationMatrix[1].x = -sin(rotation);
	rotationMatrix[1].y = cos(rotation);

	glm::mat3 V = glm::inverse(shiftMatrix * rotationMatrix * scaleMatrix);
	this->GaussianMatrix = glm::transpose(V) * V;
}

glm::vec4 GaussianSplat::operator()(glm::vec2 pos)
{
    return color * glm::clamp( (float)(magnitude * exp(0.5f * glm::dot(glm::vec3(pos, 1.0), GaussianMatrix * glm::vec3(pos, 1.0)))),
                              0.0f,1.0f);
}

float GaussianSplat::spectrum(glm::vec2 frequency)
{
	return  6.2831853 * magnitude * size.x * size.y
		* exp(0.5f *
			(powf((frequency.x *  cos(rotation) + frequency.y * sin(rotation))* size.x, 2.0f)
				+ powf((-frequency.x *  sin(rotation) + frequency.y * cos(rotation))* size.y, 2.0f))
			);
}

std::string GaussianSplat::to_string()
{
	std::stringstream gaussianConfiguration;
	gaussianConfiguration << "[ "
        << this->color.x << " " 
		<< this->color.y << " " 
		<< this->color.z << " " 
		<< this->color.w << " "
        << this->position.x << " " 
		<< this->position.y << " "
        << this->size.x << " " 
		<< this->size.y<< " "
        << this->magnitude << " " 
		<< this->rotation 
		<< " ]" << std::endl;
    
    // Matrice
//	gaussianConfiguration << "[ ";
//	for (int x = 0; x < 3; ++x) { // glm : column major matrix
//		gaussianConfiguration << "[ ";
//		for (int y = 0; y < 3; ++y) {
//			gaussianConfiguration << GaussianMatrix[x][y] << " ";
//		}
//		gaussianConfiguration << "]" << std::endl;
//	}
//	gaussianConfiguration << "]";

	return gaussianConfiguration.str();
}

std::vector<GaussianSplat*> GaussianSplat::readListFromFile(const char* filename) {
    std::vector<GaussianSplat*> list;
    
    std::ifstream reader(filename,std::fstream::in);
    
    char c = reader.get();
    while (reader.good()) {
        if(c == '[') { // Start reading current Gaussian splat
            glm::vec4 color;
            reader >> color.x;
            reader >> color.y;
            reader >> color.z;
            reader >> color.w;
            glm::vec2 position;
            reader >> position.x;
            reader >> position.y;
            glm::vec2 size;
            reader >> size.x;
            reader >> size.y;
            float magnitude, rotation;
            reader >> magnitude;
            reader >> rotation;
            list.push_back(new GaussianSplat(position, magnitude,size,rotation,color) );

        }
        c = reader.get();
    }
    return list;
}

std::vector<GaussianSplat*> GaussianSplat::readListFromSVGExtraction(const char * filename)
{
	std::vector<GaussianSplat*> list;

	std::ifstream reader(filename, std::fstream::in);

	glm::vec2 imageSize = glm::vec2(1.0, 1.0);

	char c = reader.get();
	while (reader.good()) {
		if (c == 'i') {
			c = reader.get(); // [
			reader >> imageSize.x;
			reader >> imageSize.y;
		}
		if (c == 'e') {
			c = reader.get(); // [
			glm::vec3 center = glm::vec3(0.0,0.0,1.0);
			glm::vec3 axeX = glm::vec3(0.0), axeY= glm::vec3(0.0);
			glm::mat3 transform;
			glm::vec3 color;

			reader >> center.x;
			reader >> center.y;

			reader >> axeX.x;
			reader >> axeY.y;
			
			reader >> transform[0].x;
			reader >> transform[0].y;

			reader >> transform[1].x;
			reader >> transform[1].y;

			reader >> transform[2].x;
			reader >> transform[2].y;

			reader >> color.r;
			reader >> color.g;
			reader >> color.b;

			glm::vec3 r_X = transform * (center + axeX);
			glm::vec3 r_Y = transform * (center + axeY);
			glm::vec3 o = transform * center;

			r_X = (r_X - o) / glm::vec3(imageSize,1.0);
			r_Y = (r_Y - o) / glm::vec3(imageSize, 1.0);
			o /= glm::vec3(imageSize, 1.0);
			
			float rotation = acos(glm::dot(glm::normalize(glm::vec2(r_X.x, r_X.y)), glm::vec2(1, 0)));
			glm::vec2 shift = -0.5f + glm::vec2(o.x,o.y);
			glm::vec2 scale = glm::vec2(glm::length(r_X) / 4.71f, glm::length(r_Y) / 4.71f);

			list.push_back(new GaussianSplat(shift, 1.0f, scale, rotation, glm::vec4(color,1.0f)));

		}
		
		c = reader.get();
	}
	return list;
}

