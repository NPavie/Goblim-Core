#include "GPUSpotVector.h"
#include <iostream>
#include <queue>
#include <string.h>
#include <sstream>




GaussianGPU::GaussianGPU()
	: V(glm::mat4(1.0)), color(1.0)
{}

GaussianGPU::~GaussianGPU()
{}

GaussianGPU::GaussianGPU(float magnitude, glm::vec3 shift, glm::vec3 XYZrotations, glm::vec3 scale, glm::vec3 color)
	: V(glm::mat4(1.0)), color(color, magnitude)
{
	
	glm::mat4 M = glm::mat4(1.0);
	glm::mat4 R = glm::mat4(1.0);
	glm::mat4 S = glm::mat4(1.0);
	

	M[3].x = shift.x;
	M[3].y = shift.y;
	M[3].z = shift.z;

	glm::mat4 Rx = glm::mat4(
	glm::vec4(1,0,0,0),
	glm::vec4(0, glm::cos(XYZrotations.x), glm::sin(XYZrotations.x),0),
	glm::vec4(0, -glm::sin(XYZrotations.x), glm::cos(XYZrotations.x), 0),
	glm::vec4(0,0,0,1));
	glm::mat4 Ry = glm::mat4(
	glm::vec4(glm::cos(XYZrotations.y), 0, -glm::sin(XYZrotations.y), 0),
	glm::vec4(0, 1, 0, 0),
	glm::vec4(glm::sin(XYZrotations.y), 0, glm::cos(XYZrotations.y), 0),
	glm::vec4(0, 0, 0, 1));
	glm::mat4 Rz = glm::mat4(
	glm::vec4(glm::cos(XYZrotations.z), glm::sin(XYZrotations.z), 0, 0),
	glm::vec4(-glm::sin(XYZrotations.z), glm::cos(XYZrotations.z), 0, 0),
	glm::vec4(0, 0, 1, 0),
	glm::vec4(0, 0, 0, 1));

	R = Rz * Ry * Rx;

	S[0].x = scale.x;
	S[1].y = scale.y;
	S[2].z = scale.z;

	// TO TEST, FROM MY SPOTTREE SHADER
	/*
	// Ellipse = specific quadratic iso surface, scaled along its axis
	// -> Q = (ShiftMat * RotMat * ScaleMat)^-T * (ShiftMat * RotMat * ScaleMat)^-1
	// cosines
	vec3 c = cos(rotation);
	// sines
	vec3 s = sin(rotation);
	// REMINDER : matrices are column order in glsl (a vec4 is a column)
	// This a simple predevelopped form of the ShiftMat * RotMat * ScaleMat operation (with RotMat = Rz * Ry * Rx)
	// UNSUREFLAG
	mat4 MRS = mat4( 
		scale.x * vec4((c.z * c.y), (s.z * c.y), -s.y, 0.0 ), 
		scale.y * vec4((c.z * s.y * s.x - s.z * c.x) , (s.z * s.y * s.x + c.z * c.x ), (c.y * s.x), 0.0 ),
		scale.z * vec4((c.z * s.y * c.x + s.z * s.x) , (s.z * s.y * c.x - c.z * s.x ), (c.y * c.x), 0.0),
		vec4(shift,1.0));
	*/

	V = glm::inverse(M * R * S);
	V = glm::transpose(V) * V;
	
}



DistribParam::DistribParam()
{

}

DistribParam::~DistribParam()
{}

DistribParam::DistribParam(const glm::vec3 & responseSize, const glm::vec2 & weightRange, const glm::mat3 & posMinCenterMax, const glm::vec4 & rotationsMinMax, const glm::vec3 & scaleMin, const glm::vec3 & scaleMax)
{
	int i = -1;
	int j = 0;

	for ( j = 0; j < 3; j++) data[++i] = responseSize[j];

	for ( j = 0; j < 2; j++) data[++i] = weightRange[j];
	
	for ( j = 0; j < 3; j++)
	{
		data[++i] = posMinCenterMax[j].x;
		data[++i] = posMinCenterMax[j].y;
		data[++i] = posMinCenterMax[j].z;
	}

	for ( j = 0; j < 4; j++) data[++i] = rotationsMinMax[j];

	for ( j = 0; j < 3; j++) data[++i] = scaleMin[j];

	for ( j = 0; j < 3; j++) data[++i] = scaleMax[j];

}



GPUSpotVector::GPUSpotVector()
{}

GPUSpotVector::GPUSpotVector(std::vector<Spot*> spotVector, std::vector<DistribParam*> distribVector, bool preloadToGPU)
{
	parseSpotsAndDistributionVectors(spotVector, distribVector);
	if (preloadToGPU) loadToGPU();
}

GPUSpotVector::~GPUSpotVector()
{}

void GPUSpotVector::parseSpotsAndDistributionVectors(std::vector<Spot*> spotVector, std::vector<DistribParam*> distribVector)
{
	for (int i = 0; i < spotVector.size(); ++i)
	{
		parseSpot(spotVector[i]);
	}

	for (int i = 0; i < distribVector.size(); ++i)
	{
		distribCPUArray.push_back(*distribVector[i]);
	}

	// Everything done, store size of arrays;
	sizeOfArrays temp;
	temp.constantArraySize = constantCPUArray.size();
	temp.gaussianArraySize = gaussianCPUArray.size();
	temp.harmonicArraySize = harmonicCPUArray.size();
	temp.distribArraySize = distribCPUArray.size();
	temp.spotDataArraySize = spotDataCPUArray.size();
	temp.spotIndexArraySize = spotIndexCPUArray.size();
	sizeOfArraysCPU.push_back(temp);

	globalMenu = TwNewBar("Global Menu");

	gaussians_fall = 1.0;
	gaussians_scale = 1.0;

	TwAddVarRW(globalMenu, "Gaussian Fall%", TW_TYPE_FLOAT, &(gaussians_fall), "step=0.005 min=0.01");
	TwAddVarRW(globalMenu, "Gaussians Scale%", TW_TYPE_FLOAT, &(gaussians_scale), "step=0.005");

	// Create menu to configure the arrays
	spotMenu = TwNewBar("Spots Menu");
	TwAddButton(spotMenu, "bW", NULL, NULL, "label=Weights");
	for (int i = 0; i < weightsCPUArray.size(); ++i)
	{
		std::stringstream valName = std::stringstream();
		valName << "w" << i;
		TwAddSeparator(spotMenu, valName.str().c_str(), "");;

		valName = std::stringstream();
		valName << "W_" << i;
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(weightsCPUArray[i]), "step=0.005");
	}

	TwAddButton(spotMenu, "b0", NULL, NULL, "label=Constantes");
	//TwAddSeparator(spotMenu, "s1", "");
	for (int i = 0; i < temp.constantArraySize; i++)
	{
		std::stringstream valName = std::stringstream();
		valName << "c" << i;
		TwAddSeparator(spotMenu, valName.str().c_str(), "");;

		valName = std::stringstream();
		valName << "C_" << i;
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(constantCPUArray[i].constanteValue), "step=0.005");

	}
	TwAddSeparator(spotMenu, "s2", "");
	TwAddButton(spotMenu, "b1", NULL, NULL, "label=Gaussiennes");
	//TwAddSeparator(spotMenu, "s3", "");
	for (int i = 0; i < temp.gaussianArraySize; i++)
	{
		std::stringstream valName = std::stringstream();

		valName << "g" << i;
		TwAddSeparator(spotMenu, valName.str().c_str(), "");;

		valName = std::stringstream();

		valName << "S_" << i << "_scaleX";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(gaussianCPUArray[i].scale.x), "step=0.005");
		valName = std::stringstream();
		valName << "S_" << i << "_scaleY";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(gaussianCPUArray[i].scale.y), "step=0.005");
		valName = std::stringstream();
		valName << "S_" << i << "_scaleZ";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(gaussianCPUArray[i].scale.z), "step=0.005");
		valName = std::stringstream();
		valName << "S_" << i << "_Falloff";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(gaussianCPUArray[i].scale.w), "step=0.005");
		valName = std::stringstream();


		valName << "M_" << i << "_posX";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(gaussianCPUArray[i].shift.x), "step=0.005");
		valName = std::stringstream();
		valName << "M_" << i << "_posY";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(gaussianCPUArray[i].shift.y), "step=0.005");
		valName = std::stringstream();
		valName << "M_" << i << "_posZ";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(gaussianCPUArray[i].shift.z), "step=0.005");
		valName = std::stringstream();

		valName << "R_" << i << "_aroundX";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(gaussianCPUArray[i].aroundXYZrotations.x), "step=0.005");
		valName = std::stringstream();
		valName << "R_" << i << "_aroundY";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(gaussianCPUArray[i].aroundXYZrotations.y), "step=0.005");
		valName = std::stringstream();
		valName << "R_" << i << "_aroundZ";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(gaussianCPUArray[i].aroundXYZrotations.z), "step=0.005");
		valName = std::stringstream();

		valName << "C_" << i << "_red";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(gaussianCPUArray[i].color.x), "step=0.005");
		valName = std::stringstream();
		valName << "C_" << i << "_green";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(gaussianCPUArray[i].color.y), "step=0.005");
		valName = std::stringstream();
		valName << "C_" << i << "_blue";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(gaussianCPUArray[i].color.z), "step=0.005");
		valName = std::stringstream();


	}
	TwAddSeparator(spotMenu, "s3", "");
	TwAddButton(spotMenu, "b2", NULL, NULL, "label=Harmonics");
	//TwAddSeparator(spotMenu, "s4", "");
	for (int i = 0; i < temp.harmonicArraySize; i++)
	{
		std::stringstream valName = std::stringstream();

		valName << "h" << i;
		TwAddSeparator(spotMenu, valName.str().c_str(), "");;
		valName = std::stringstream();

		valName << "H_" << i << "_frequency";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(harmonicCPUArray[i].thetaPhiFrequency.x), "step=0.005");

		valName = std::stringstream();
		valName << "H_" << i << "_Theta";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(harmonicCPUArray[i].thetaPhiFrequency.y), "step=0.005");

		valName = std::stringstream();
		valName << "H_" << i << "_Phi";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(harmonicCPUArray[i].thetaPhiFrequency.z), "step=0.005");

	}

	TwAddSeparator(spotMenu, "s4", "");
	TwAddButton(spotMenu, "b3", NULL, NULL, "label=Distributions");
	//TwAddSeparator(spotMenu, "s5", "");
	for (int i = 0; i < temp.distribArraySize; i++)
	{
		std::stringstream valName = std::stringstream();

		valName << "d" << i;
		TwAddSeparator(spotMenu, valName.str().c_str(), "");;
		valName = std::stringstream();

		int j = 0;
		// response size
		valName << "Data_" << i << "_respX";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;

		valName = std::stringstream();
		valName << "Data_" << i << "_respY";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;
		valName = std::stringstream();
		valName << "Data_" << i << "_ImpAxe";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "min=1.0 step=1.0");
		j++;

		valName = std::stringstream();
		valName << "Data_" << i << "_Wmin";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;
		valName = std::stringstream();
		valName << "Data_" << i << "_Wmax";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;

		valName = std::stringstream();
		valName << "Data_" << i << "_posMinX";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;
		valName = std::stringstream();
		valName << "Data_" << i << "_posMinY";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;
		valName = std::stringstream();
		valName << "Data_" << i << "_posMinZ";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;

		valName = std::stringstream();
		valName << "Data_" << i << "_posCenterX";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;
		valName = std::stringstream();
		valName << "Data_" << i << "_posCenterY";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;
		valName = std::stringstream();
		valName << "Data_" << i << "_posCenterZ";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;

		valName = std::stringstream();
		valName << "Data_" << i << "_posMaxX";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;
		valName = std::stringstream();
		valName << "Data_" << i << "_posMaxY";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;
		valName = std::stringstream();
		valName << "Data_" << i << "_posMaxZ";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;

		valName = std::stringstream();
		valName << "Data_" << i << "_rotateMinX";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;
		valName = std::stringstream();
		valName << "Data_" << i << "_rotateMinY";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;
		valName = std::stringstream();
		valName << "Data_" << i << "_rotateMaxX";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;
		valName = std::stringstream();
		valName << "Data_" << i << "_rotateMaxY";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;

		valName = std::stringstream();
		valName << "Data_" << i << "_scaleMinX";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;
		valName = std::stringstream();
		valName << "Data_" << i << "_scaleMinY";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;
		valName = std::stringstream();
		valName << "Data_" << i << "_scaleMinZ";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;

		valName = std::stringstream();
		valName << "Data_" << i << "_scaleMaxX";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;
		valName = std::stringstream();
		valName << "Data_" << i << "_scaleMaxY";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;
		valName = std::stringstream();
		valName << "Data_" << i << "_scaleMaxZ";
		TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.005");
		j++;

		//for (int j = 0; j < 24; j++)
		//{
		//	std::stringstream valName = std::stringstream();
		//	valName << "Data_" << i << "_" << j;
		//	TwAddVarRW(spotMenu, valName.str().c_str(), TW_TYPE_FLOAT, &(distribCPUArray[i].data[j]), "step=0.1");
		//}

	}

}

void GPUSpotVector::parseSpot(Spot* sp)
{
	// parsing spot functions and storing them in the GPU tree vectors
	spotIndexCPUArray.push_back(spotDataCPUArray.size());			// Index to first data (NbFunctions)
	weightsCPUArray.push_back(sp->spotWeight);						// Weight of the generated spot noise ( should be 1.0 / maximum amplitude of the generated noise i think)
	spotDataCPUArray.push_back(sp->getFunctionsVector()->size());	// Nb Functions in spot
	for (int i = 0; i < sp->getFunctionsVector()->size(); i++)
	{
		Function temp = sp->getFunctionsVector()->at(i);
		switch (sp->getFunctionsVector()->at(i).typeId)
		{
		case GAUSSIAN:
		default:
			temp.paramId = gaussianCPUArray.size();
			gaussianCPUArray.push_back(sp->getGaussiansVector()->at(sp->getFunctionsVector()->at(i).paramId));
			break;
		case HARMONIC:
			temp.paramId = harmonicCPUArray.size();
			harmonicCPUArray.push_back(sp->getHarmonicsVector()->at(sp->getFunctionsVector()->at(i).paramId));
			break;
		case CONSTANT:
			temp.paramId = constantCPUArray.size();
			constantCPUArray.push_back(sp->getConstantsVector()->at(sp->getFunctionsVector()->at(i).paramId));
			break;
		}
		spotDataCPUArray.push_back(temp.operand);
		spotDataCPUArray.push_back(temp.typeId);
		spotDataCPUArray.push_back(temp.paramId);
		
	}
		
}

// Loading every data to GPU
void GPUSpotVector::loadToGPU()
{


	gaussianDataGPUarray = new GPUArray<GaussianGPU>( reducedGaussiansArray(gaussianCPUArray));
	harmonicDataGPUarray = new GPUArray<Harmonic>(harmonicCPUArray);
	constantDataGPUarray = new GPUArray<Constant>(constantCPUArray);
	distribDataGPUarray = new GPUArray<DistribParam>(distribCPUArray);
	spotDataGPUArray = new GPUArray<int>(spotDataCPUArray);
	spotIndexGPUArray = new GPUArray<int>(spotIndexCPUArray);
	sizeOfArraysGPU = new GPUArray<sizeOfArrays>(sizeOfArraysCPU);
	weightsGPUArray = new GPUArray<float>(weightsCPUArray);

	int i = 2;
	gaussianDataBuffer		= gaussianDataGPUarray->createGPUBuffer("gaussianDataBuffer",++i);
	harmonicDataBuffer		= harmonicDataGPUarray->createGPUBuffer("harmonicDataBuffer",++i);
	constantDataBuffer		= constantDataGPUarray->createGPUBuffer("constantDataBuffer",++i);
	distribDataBuffer		= distribDataGPUarray->createGPUBuffer("distribDataBuffer", ++i);
	spotDataBuffer			= spotDataGPUArray->createGPUBuffer("spotDataBuffer",++i);
	spotIndexBuffer			= spotIndexGPUArray->createGPUBuffer("spotIndexBuffer", ++i);
	sizeOfArraysBuffer		= sizeOfArraysGPU->createGPUBuffer("sizeOfArraysBuffer", ++i);
	weightsDataBuffer		= weightsGPUArray->createGPUBuffer("weightsDataBuffer", ++i);
	
	
	gaussianDataBuffer->bind();
	harmonicDataBuffer->bind();
	constantDataBuffer->bind();
	distribDataBuffer->bind();
	spotDataBuffer->bind();
	spotIndexBuffer->bind();
	sizeOfArraysBuffer->bind();
	weightsDataBuffer->bind();
	
	
	
}

int GPUSpotVector::size()
{
	return sizeOfArraysCPU[0].spotIndexArraySize;
}
	
void GPUSpotVector::reloadToGPU()
{

	//gaussianDataBuffer->release();
	//harmonicDataBuffer->release();
	//constantDataBuffer->release();
	//distribDataBuffer->release();
	//spotDataBuffer->release();
	//spotsTreeNodeDataBuffer->release();
	//spotIndexBuffer->release();
	//spotsTreeNodeIndexBuffer->release();
	//sizeOfArraysBuffer->release();

	
	gaussianDataGPUarray->fillFromCPUArray(reducedGaussiansArray(gaussianCPUArray));
	harmonicDataGPUarray->fillFromCPUArray(harmonicCPUArray);
	constantDataGPUarray->fillFromCPUArray(constantCPUArray);
	distribDataGPUarray->fillFromCPUArray(distribCPUArray);
	spotDataGPUArray->fillFromCPUArray(spotDataCPUArray);
	spotIndexGPUArray->fillFromCPUArray(spotIndexCPUArray);
	sizeOfArraysGPU->fillFromCPUArray(sizeOfArraysCPU);
	weightsGPUArray->fillFromCPUArray(weightsCPUArray);

	gaussianDataGPUarray->updateGPUBuffer(gaussianDataBuffer);
	harmonicDataGPUarray->updateGPUBuffer(harmonicDataBuffer);
	constantDataGPUarray->updateGPUBuffer(constantDataBuffer);
	distribDataGPUarray->updateGPUBuffer(distribDataBuffer);
	spotDataGPUArray->updateGPUBuffer(spotDataBuffer);
	spotIndexGPUArray->updateGPUBuffer(spotIndexBuffer);
	weightsGPUArray->updateGPUBuffer(weightsDataBuffer);

	gaussianDataBuffer->bind();
	harmonicDataBuffer->bind();
	constantDataBuffer->bind();
	distribDataBuffer->bind();
	spotDataBuffer->bind();
	spotIndexBuffer->bind();
	sizeOfArraysBuffer->bind();
	weightsDataBuffer->bind();

}


std::vector<GaussianGPU> GPUSpotVector::reducedGaussiansArray(std::vector<Gaussian> cpuArray)
{
	std::vector<GaussianGPU> newArray;
	for (int i = 0; i < cpuArray.size(); i++)
	{
		newArray.push_back(GaussianGPU(cpuArray[i].scale.w * gaussians_fall,
			glm::vec3(cpuArray[i].shift.x, cpuArray[i].shift.y, cpuArray[i].shift.z),
			glm::vec3(cpuArray[i].aroundXYZrotations.x, cpuArray[i].aroundXYZrotations.y, cpuArray[i].aroundXYZrotations.z),
			glm::vec3(cpuArray[i].scale.x, cpuArray[i].scale.y, cpuArray[i].scale.z) * gaussians_scale,
			glm::vec3(cpuArray[i].color)));
	}
	return newArray;
}