#pragma once
#include "Spot.h"

#include "GPUResources/Buffers/GPUArray.hpp"

#include <AntTweakBar/AntTweakBar.h>

/* FUNCTIONS PARAMETERS FOR SPOT EVALUATION */

/** @brief	Configuration class for the Gaussian kernel equation : 
			G(X) = K e^(-0.5 * X^t.V.X) with the V matrix precomputed in CPU 
*/
class GaussianGPU
{
public:
	/** @brief Default gaussian kernel (no shift, no rotation, scale and magnitude = 1.0) */
	GaussianGPU();
	~GaussianGPU();

	/** @brief	Gaussian kernel configuration
	@param magnitude	magnitude K of the gaussian kernel
	@param shift		translation of the gaussian kernel in its evaluation space
	@param XYZrotations	rotations of the gaussian kernel around its 3 axis
	@param scale		scaling of the gaussian space
	*/
	GaussianGPU(float magnitude, glm::vec3 shift, glm::vec3 XYZrotations, glm::vec3 scale,glm::vec3 color = glm::vec3(1.0f));

	glm::mat4 V;
	glm::vec4 color; //color.rgb, magnitude = color.a
};


/** @brief	Spots distribution configuration class.
			The configuration defines several constraint and settings for the distribution process of spots
			- The spot response size (size of the kernel axes, used to compute the grid)
			- The range of possible randomly generated weight
			- The random position range defined as 3 vector such as the position is generated in [center - min, center + max]
			- The random rotation angles range (for now only 2 axes are considered)
			- The minimum scale over the 3 spot axes
			- The maximum scale over the 3 spot axes
*/
class DistribParam
{
public:
	/** @brief default distribution constructor (<b> do not use this : the default distribution has not been defined yet</b>)*/
	DistribParam();
	~DistribParam();
	/** @brief Distribution configuration contructor
		@param responseSize		Size of the kernel axes when distributed on the surface and number of impulses
		@param weightRange		The range of possible randomly generated weight
		@param posMinCenterMax	The random position range defined as 3 vector such as the position is generated in [center - min, center + max]
		@param rotationsMinMax	The random rotation angles range (for now only 2 axes are considered)
		@param scaleMin			The minimum scale over the 3 spot axes
		@param scaleMax			The maximum scale over the 3 spot axes
	*/
	DistribParam(const glm::vec3 & responseSize, const glm::vec2 & weightRange, const glm::mat3 & posMinCenterMax, const glm::vec4 & rotationsMinMax, const glm::vec3 & scaleMin, const glm::vec3 & scaleMax);

	// IN DETAILS (with index) : 
	// vec3 responseSize 0 1 2
	//
	// - random weight range
	//float w_min; 3 
	//float w_max; 4
	//
	// - random position range
	//vec3 p_min; 5 6 7
	//vec3 p_center; 8 9 10
	//vec3 p_max; 11 12 13
	//
	// - random rotation range
	//vec2 rotate_min; 14 15 
	//vec2 rotate_max; 16 17
	//
	// - random scale range
	//vec3 scale_min; 18 19 20
	//vec3 scale_max; 21 22 23
	float data[24];
	// 21 float
	// + taille de la réponse du noyau associé à la distrib
	
};


struct sizeOfArrays
{
	int gaussianArraySize;
	int harmonicArraySize;
	int constantArraySize;
	int distribArraySize;
	int spotDataArraySize;
	int spotIndexArraySize;
};
/* -------------------------------------- */


/** Spots structure to transfert to the GPU */
class GPUSpotVector
{

public:
	GPUSpotVector();
	GPUSpotVector(std::vector<Spot*> spotVector, std::vector<DistribParam*> distribVector, bool preloadToGPU = true);
	~GPUSpotVector();


	void parseSpotsAndDistributionVectors(std::vector<Spot*> spotVector, std::vector<DistribParam*> distribVector);

	void parseSpot(Spot* sp);
	
	/** @brief  */
	void loadToGPU();

	int size();

	void reloadToGPU();

	std::vector<TwBar*> spotsMenu;

	TwBar* spotMenu;

	TwBar* globalMenu;

	float gaussians_fall;
	float gaussians_scale;

private:

	std::vector<GaussianGPU> reducedGaussiansArray(std::vector<Gaussian> cpuArray);
	
	std::vector<Gaussian>	gaussianCPUArray;
	std::vector<Harmonic>	harmonicCPUArray;
	std::vector<Constant>	constantCPUArray;
	std::vector<DistribParam>	distribCPUArray;
	std::vector<int> spotDataCPUArray;
	std::vector<int> spotIndexCPUArray;
	std::vector<sizeOfArrays> sizeOfArraysCPU;
	std::vector<float>		weightsCPUArray;

	GPUArray<GaussianGPU>*	gaussianDataGPUarray;
	GPUArray<Harmonic>*	harmonicDataGPUarray;
	GPUArray<Constant>*	constantDataGPUarray;
	GPUArray<DistribParam>*		distribDataGPUarray;
	GPUArray<int>*				spotDataGPUArray;
	GPUArray<int>*				spotIndexGPUArray;
	GPUArray<sizeOfArrays>*		sizeOfArraysGPU;
	GPUArray<float>*		weightsGPUArray;
	
	GPUBuffer* gaussianDataBuffer;
	GPUBuffer* harmonicDataBuffer;
	GPUBuffer* constantDataBuffer;
	GPUBuffer* distribDataBuffer;
	GPUBuffer* spotDataBuffer;
	GPUBuffer* spotIndexBuffer;
	GPUBuffer* sizeOfArraysBuffer;
	GPUBuffer* weightsDataBuffer;



};
