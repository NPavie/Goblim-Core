#pragma once
#include <glm/glm.hpp>

#include <vector>

/** @brief	Configuration class for the Gaussian kernel equation : G(X) = K e^(-0.5 * X^t.V.X)
where K is the magnitude, X is the evaluated vector and V is the quadratic matrix of
the gaussian constructed from shift, rotations and scaling operations
*/
class Gaussian
{
public:
	/** @brief Default gaussian kernel (no shift, no rotation, scale and magnitude = 1.0) */
	Gaussian();
	/** @brief Gaussian kernel configuration
	@param magnitude	magnitude K of the gaussian kernel
	@param shift		translation of the gaussian kernel in its evaluation space
	@param XYZrotations	rotations of the gaussian kernel around its 3 axis
	@param scale		scaling of the gaussian space
	*/
	Gaussian(float magnitude, glm::vec3 shift, glm::vec3 XYZrotations, glm::vec3 scale,glm::vec3 color = glm::vec3(1.0));
	
	~Gaussian();
	
	glm::mat4 getFinalMatrix();

	float getDeterminant();

	float operator() (const glm::vec3 & p);

	glm::vec4 shift;
	glm::vec4 aroundXYZrotations;
	glm::vec4 scale; // Scale on X, Y, Z and gaussian magnitude
	glm::vec3 color;


};

/** @brief	Configuration class for the harmonic equation : H(x,y) = cos(2*PI*F * (x * cos(theta) * sin(phi) + y * sin(theta) * sin(phi) + z * cos(phi) ) )
where F is the harmonic frequency, (x,y) is the evaluated point and theta is the harmonic orientation.
*/
class Harmonic
{
public:

	Harmonic();
	~Harmonic();
	Harmonic(float frequency, glm::vec2 sphericalCoord);

	float operator() (const glm::vec3 & p);

	glm::vec4 thetaPhiFrequency; // its ugly, but its ALIV... aligned => x = theta, y = phi, z = frequency, w = 0; 
};
/** @brief	Configuration class for the constant function equation :
C(X) = c
*/
class Constant
{
public:
	/** @brief Default constant function, with c = 1.0 */
	Constant();
	~Constant();
	Constant(float constanteValue);

	float operator() (const glm::vec3 & p);

	float constanteValue;
};


/** @brief type of function that can composed a spot : GAUSSIAN,HARMONIC,CONSTANT */
enum function_type { GAUSSIAN, HARMONIC, CONSTANT };

/** Definition of a basis function */
struct Function
{
	/** @brief Type of the basis function (GAUSSIAN,HARMONIC,CONSTANT) */
	function_type typeId;
	/** @brief index of the function parameters in the parameters array */
	int paramId;
	/** @brief operand index : (0 = ADD, 1 = SUB, 2 = MULT)  */
	int operand;
};


#ifndef OPERAND_ENUM
#define OPERAND_ENUM
enum operand { ADD, SUB, MULT, DIV };
#endif

/** @brief Procedural spot (CPU data): density function designed by compsition of basis function (Gaussian or Harmonic or Constant) */
class Spot
{
public:
	/** @brief Default spot : an empty profile (no density function set) with a noise default ponderation set to 1 */
	Spot(float spotWeight = 1.0);

	~Spot();
	/** @brief Add a Gaussian function to the spot composition, combined with an operator for evaluation of the function (Spot [+,-,*]= GaussianFunction(G) )
	*/
	void addFunction(operand ope, Gaussian* G);

	void removeGaussianFunction(int nb);
	
	/** @brief Add an Harmonic function to the spot composition, combined with an operator for evaluation of the function (Spot [+,-,*]= HarmonicFunction(H) )
	*/
	void addFunction(operand ope, Harmonic* H);
	
	/** @brief Add a Constant function to the spot composition, combined with an operator for  evaluation of the function (Spot [+,-,*]= Constant(C) )
	*/
	void addFunction(operand ope, Constant* C);

	/** @brief evaluate the spot density at a point p of the evaluation space (result is clamped in [0,1])
	*/
	float operator() (const glm::vec3 & p);

	void loadFromSVGParserResult(const char* filename);

	void loadFromEllipsesFile(const char* elipseFile);

	void updateGaussianSpotColor(glm::vec3 rgbColor);

	std::vector<Gaussian>* getGaussiansVector();

	std::vector<glm::mat4>* getGaussianFinalVector();

	std::vector<Harmonic>* getHarmonicsVector();

	std::vector<Constant>* getConstantsVector();

	std::vector<Function>* getFunctionsVector();


	float spotWeight;

protected:
	/** @brief List of basis function that create the spot */
	std::vector<Function>	internal_function_list;

	/** @brief Gaussian functions parameters array */
	std::vector<Gaussian>	gaussians;
	/** @brief Harmonic functions parameters array */
	std::vector<Harmonic>	harmonics;
	/** @brief Constant functions parameters array */
	std::vector<Constant>	constants;


	bool gaussiansMatricesComputed;
	/** @brief Gaussian functions precomputed matrices (constructed by getGaussianFinalVector) */
	std::vector<glm::mat4> gaussiansFinal;

};