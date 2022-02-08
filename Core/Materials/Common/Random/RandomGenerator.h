#ifndef _RandomGenerator_h
#define _RandomGenerator_h

#include <glm/glm.hpp>

class RandomGenerator /* Random generator using morton code */
{

public:
	~RandomGenerator();


	/*
	*	uninitialized Random Generator 
	*/
	RandomGenerator();
	

	/*
	*	Random Generator initialized with a integer identifier
	*/
	RandomGenerator(float seed);

	/*
	*	Random Generator initialized with a 2D cell identifier
	*/
	RandomGenerator(glm::vec2 seedCell);

	/*
	*	Random Generator initialized with a 3D cell identifier
	*/
	RandomGenerator(glm::vec3 seedCell);


	/*
	*	@brief starting the number generation from an integer identifier
	*/
	void initRandom(float seed);
	
	/*
	*	@brief starting the number generation from a 2D cell identifier
	*/
	void initRandom(glm::vec2 seedCell);

	/*
	*	@brief starting the number generation from a 3D cell identifier (Not working for now)
	*/
	void initRandom(glm::vec3 seedCell);

		
	/*
	*	@brief Standard number generator
	*	@return a float number between 0 and 1
	*/
	float random();

	/*
	*	@brief Standard number generator
	*	@return a vec2 with each coordinate between 0 and 1
	*/
	glm::vec2 randomVec2();

	/*
	*	@brief Standard number generator
	*	@return a vec3 with each coordinate between 0 and 1
	*/
	glm::vec3 randomVec3();

	/*
	*	@brief Extended number generation
	*	@return a float number between min and max
	*/
	float randomIn(float min, float max);

	/*
	*	@brief Extended number generation
	*	@return a vec2 number between min and max
	*/
	glm::vec2 randomIn(glm::vec2 min, glm::vec2 max);

	/*
	*	@brief Extended number generation
	*
	*	@return a vec3 number between min and max
	*/
	glm::vec3 randomIn(glm::vec3 min, glm::vec3 max);



private:
	unsigned int seed;
	void seeding(unsigned int s);

	/* Classic morton code */

	// "Insert" a 0 bit after each of the 16 low bits of x
	unsigned int Part1By1(unsigned int x);

	// "Insert" two 0 bits after each of the 10 low bits of x
	unsigned int Part1By2(unsigned int x);

	// Morton code on 2D units
	unsigned int morton(unsigned int x, unsigned int y);
	unsigned int morton2D(unsigned int x, unsigned int y);

	// Morton code on 3D units
	unsigned int morton3D(unsigned int x, unsigned int y, unsigned int z);
	unsigned int morton3D(glm::vec3 v);

	unsigned int poisson(float mean);

	unsigned int next();

};

#endif