#include "Spot.h"

#include <fstream>

#ifndef M_PI
#define M_PI 3.14159265359
#endif

Gaussian::Gaussian()
	: shift(0.0), aroundXYZrotations(0.0), scale(1.0)
{}

Gaussian::~Gaussian()
{}

Gaussian::Gaussian(float magnitude, glm::vec3 shift, glm::vec3 XYZrotations, glm::vec3 scale, glm::vec3 color)
	: shift(shift, 0.0), aroundXYZrotations(XYZrotations, 0.0), scale(scale, magnitude), color(color)
{}


glm::mat4 Gaussian::getFinalMatrix()
{
	glm::mat4 M = glm::mat4(1.0);
	glm::mat4 R = glm::mat4(1.0);
	glm::mat4 S = glm::mat4(1.0);

	M[3].x = shift.x;
	M[3].y = shift.y;
	M[3].z = shift.z;

	glm::mat4 Rx = glm::mat4(
		glm::vec4(1, 0, 0, 0),
		glm::vec4(0, glm::cos(aroundXYZrotations.x), glm::sin(aroundXYZrotations.x), 0),
		glm::vec4(0, -glm::sin(aroundXYZrotations.x), glm::cos(aroundXYZrotations.x), 0),
		glm::vec4(0, 0, 0, 1));
	glm::mat4 Ry = glm::mat4(
		glm::vec4(glm::cos(aroundXYZrotations.y), 0, -glm::sin(aroundXYZrotations.y), 0),
		glm::vec4(0, 1, 0, 0),
		glm::vec4(glm::sin(aroundXYZrotations.y), 0, glm::cos(aroundXYZrotations.y), 0),
		glm::vec4(0, 0, 0, 1));
	glm::mat4 Rz = glm::mat4(
		glm::vec4(glm::cos(aroundXYZrotations.z), glm::sin(aroundXYZrotations.z), 0, 0),
		glm::vec4(-glm::sin(aroundXYZrotations.z), glm::cos(aroundXYZrotations.z), 0, 0),
		glm::vec4(0, 0, 1, 0),
		glm::vec4(0, 0, 0, 1));

	R = Rz * Ry * Rx;

	S[0].x = scale.x;
	S[1].y = scale.y;
	S[2].z = scale.z;

	glm::mat4 V = glm::inverse(M * R * S);
	V = glm::transpose(V) * V;
	return V;
}

float Gaussian::getDeterminant()
{
	glm::mat4 V = getFinalMatrix();
	return glm::determinant(V);
}



float Gaussian::operator()(const glm::vec3 & p)
{
	glm::mat4 V = getFinalMatrix();

	glm::vec4 p_ = glm::vec4(p, 1.0);

	return scale.w * expf(-0.5f * glm::dot(p_, V * p_));
}



Harmonic::Harmonic()
	:thetaPhiFrequency(0.0, 0.0, 1.0, 0.0)
{}

Harmonic::~Harmonic()
{}

Harmonic::Harmonic(float frequency, glm::vec2 sphericalCoord)
	:thetaPhiFrequency(sphericalCoord, frequency, 0.0)
{}

float Harmonic::operator()(const glm::vec3 & p)
{
	return cos(2.0f * M_PI * thetaPhiFrequency.z * 
		(p.x * cos(thetaPhiFrequency.x) * sin(thetaPhiFrequency.y) 
		+ p.y * sin(thetaPhiFrequency.x) * sin(thetaPhiFrequency.y) 
		+ p.z * cos(thetaPhiFrequency.y)
		)
	);
}

Constant::Constant()
	: constanteValue(1.0)
{}

Constant::~Constant()
{}

Constant::Constant(float constanteValue)
	:constanteValue(constanteValue)
{}

float Constant::operator()(const glm::vec3 & p)
{
	return constanteValue;
}



Spot::Spot(float spotWeight)
	:gaussiansMatricesComputed(false),spotWeight(spotWeight)
{}

Spot::~Spot()
{}

void Spot::addFunction(operand ope, Gaussian* G)
{

	gaussians.push_back(Gaussian((*G)));
	Function toKeep;
	toKeep.typeId = GAUSSIAN;
	toKeep.paramId = gaussians.size() - 1;
	toKeep.operand = ope;
	internal_function_list.push_back(toKeep);

}

void Spot::removeGaussianFunction(int nb)
{
	if (nb < gaussians.size())
	{
		// Erase the gaussian params
		gaussians.erase(gaussians.begin() + nb);
		int id_function = 0;
		for (int i = 0; i < internal_function_list.size(); ++i) {
			if (internal_function_list[i].paramId == nb && internal_function_list[i].typeId == GAUSSIAN) {
				// Erase the corresponding gaussian function in function list
				internal_function_list.erase(internal_function_list.begin() + i);
				id_function = i;
				break; // end of suppression
			}
		}
		// Update the next gaussian function (param = param-1 si type == gaussian)
		for (int i = id_function; i < internal_function_list.size(); ++i) {
			if (internal_function_list[i].typeId == GAUSSIAN) {
				internal_function_list[i].paramId -= 1;
			}
		}
	}
}

void Spot::addFunction(operand ope, Harmonic* H)
{

	harmonics.push_back(Harmonic((*H)));
	Function toKeep;
	toKeep.typeId = HARMONIC;
	toKeep.paramId = harmonics.size() - 1;
	toKeep.operand = ope;
	internal_function_list.push_back(toKeep);
}

void Spot::addFunction(operand ope, Constant* C)
{

	constants.push_back(Constant((*C)));
	Function toKeep;
	toKeep.typeId = CONSTANT;
	toKeep.paramId = constants.size() - 1;
	toKeep.operand = ope;
	internal_function_list.push_back(toKeep);

}

float Spot::operator()(const glm::vec3 & p)
{
	float res = 0.0f;
	for (int i = 0; i < internal_function_list.size(); ++i){
		float operation = 0.0f;
		switch (internal_function_list[i].typeId)
		{
		case function_type::CONSTANT:
			operation = constants[i](p);
			break;
		case function_type::GAUSSIAN:
			operation = gaussians[i](p);
			break;
		case function_type::HARMONIC:
			operation = harmonics[i](p);
			break;
		default:
			break;
		}
		switch (internal_function_list[i].operand) {
		case operand::ADD:
			res += operation;
			break;
		case operand::MULT:
			res *= operation;
			break;
		case operand::SUB:
			res -= operation;
			break;
		case operand::DIV:
			res /= operation;
			break;
		default: 
			break;
			
		}
	}
	return glm::clamp(res,0.0f,1.0f);
}

void Spot::loadFromSVGParserResult(const char* filename)
{
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
			glm::vec3 center = glm::vec3(0.0, 0.0, 1.0);
			glm::vec3 axeX = glm::vec3(0.0), axeY = glm::vec3(0.0);
			glm::mat3 transform = glm::mat3(1.0);
			glm::vec3 color;
			float scaleImage = imageSize.x > imageSize.y ? imageSize.x : imageSize.y;

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

			center /= scaleImage;
			center.z = 1.0;
			axeX /= scaleImage;
			axeY /= scaleImage;

			// translation dans l'espace courant de la gaussienne 
			// (inkscape met les points X et Y en O-Rx et O-Ry)
			
			//if (transform[1].x < transform[0].y) {
			//	float temp = transform[0].y;
			//	transform[0].y = transform[1].x;
			//	transform[1].x = temp;
			//}

			// Passage dans l'espace de la vue
			center = transform * center;
			glm::vec3 Vx = transform * axeX;
			glm::vec3 Vy = transform * axeY;

			// Mirroir sur Y
			center.y = 1.0 - center.y;
			Vx.y = -Vx.y;
			Vy.y = -Vy.y;
			
			float angleWithX = glm::sign(-transform[0].y) * glm::acos(transform[0].x);

			glm::vec2 shift = -0.5f + glm::vec2(center.x, center.y);
			glm::vec2 scale = glm::vec2(glm::length(Vx), glm::length(Vy));

			this->addFunction(ADD, new Gaussian(0.5f, glm::vec3(shift, 0.0), glm::vec3(0.0f, 0.0f, angleWithX), glm::vec3(scale, 1.0f), color));

		}

		c = reader.get();
	}

}

void Spot::loadFromEllipsesFile(const char * elipseFile)
{

	std::ifstream reader(elipseFile, std::fstream::in);

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
			glm::vec2 center = glm::vec2(0.0);
			glm::vec2 size = glm::vec2(0.0);
			float angle;			
			glm::vec3 color;

			float scaleImage = imageSize.x > imageSize.y ? imageSize.x : imageSize.y;

			reader >> center.x;
			reader >> center.y;

			reader >> size.x;
			reader >> size.y;

			reader >> angle;

			reader >> color.r;
			reader >> color.g;
			reader >> color.b;

			center /= scaleImage;
			center.y = 1.0 - center.y;

			size /= (2.0 * scaleImage);

			color /= 255.0f;
			
			if (color.r + color.g + color.b > 0.0 && color.r + color.g + color.b < 3.0) {
				glm::vec2 shift = (-0.5f + glm::vec2(center.x, center.y)) * 1.1f;
				size *= 1.1f;
				this->addFunction(ADD, new Gaussian(0.5f, glm::vec3(shift, 0.0), glm::vec3(0.0f, 0.0f, -angle * M_PI / 180.0f), glm::vec3(size, 1.0f), color));
			}

		}

		c = reader.get();
	}

}

void Spot::updateGaussianSpotColor(glm::vec3 rgbColor)
{
	for (int i = 0; i < gaussians.size(); ++i) {
		gaussians[i].color = rgbColor;
	}
}

std::vector<Gaussian>* Spot::getGaussiansVector()
{
	return &gaussians;
}

std::vector<glm::mat4>* Spot::getGaussianFinalVector()
{
	if (gaussiansMatricesComputed) return &gaussiansFinal;
	for (int i = 0; i < gaussians.size(); ++i)
	{
		gaussiansFinal.push_back(gaussians[i].getFinalMatrix());
	}
	return &gaussiansFinal;
}

std::vector<Harmonic>* Spot::getHarmonicsVector()
{
	return &harmonics;
}

std::vector<Constant>* Spot::getConstantsVector()
{
	return &constants;
}

std::vector<Function>* Spot::getFunctionsVector()
{
	return &internal_function_list;
}

