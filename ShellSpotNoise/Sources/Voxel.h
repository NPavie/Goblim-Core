#pragma once

#include <glm/glm.hpp>
#include <vector>
#include "Engine/Base/GeometricModel.h"
#include "Engine/OpenGL/BoundingBox/BoundingBoxModelGL.h"
#include "Engine/OpenGL/v4/GLProgramPipeline.h"

// structure de donnée d'un voxel
struct VoxelData
{
	glm::vec4 position;
	glm::vec4 normale;
	glm::vec4 tangente;
	glm::vec4 coord_texture;
	glm::vec4 coord_surface; // UVW de l'élément de surface associé + distance à cet élément de surface
};

class Voxel
{
public:
	Voxel();
	Voxel(VoxelData & toCopy);
	Voxel(
		const glm::vec4 & position, 
		const glm::vec4 & normale, 
		const glm::vec4 & tangente, 
		const glm::vec4 & coord_texture, 
		const glm::vec4 & coord_surface);

	~Voxel();

	VoxelData*	getAdressOfData();
	VoxelData	getCopyOfData();

	void changePosition(const glm::vec4 & position);
	void changeNormale(const glm::vec4 & normale);
	void changeTangente(const glm::vec4 & tangente);
	void changeCoord_texture(const glm::vec4 & coord_texture);
	void changeCoord_surface(const glm::vec4 & coord_surface);

protected:
	VoxelData data;
};


class VoxelVector : public std::vector<Voxel>
{
public:
	VoxelVector();
	VoxelVector(
		GeometricModel* fromGeometry, 
		int subdivision = 8, 
		float shellHeight = 1.0);

	void compute(
		GeometricModel* fromGeometry, 
		int subdivision, 
		float shellHeight);

	VoxelVector(const char* fileName);

	// Return a ptr to GeometricBoundingBox
	GeometricBoundingBox* getGeometricBoundingBox();

	void saveInFile(const char* fileName);

	void loadFromFile(const char* fileName);

	float voxelHalfSize;

protected:
	bool computeDone;

	std::vector<glm::vec3> listExtrudedVertex;

	glm::vec3 minCoord;
	glm::vec3 maxCoord;
	GeometricBoundingBox* AABB;

	

};

// GPU structure
struct VoxelArray
{
	glm::vec4 size;
	Voxel bufferContent[];
};

class GPUVoxelVector : public VoxelVector
{
public:
	GPUVoxelVector();
	GPUVoxelVector(
		GeometricModel* fromGeometry, 
		int SSBO_binding = 4, 
		int subdivision = 8, 
		float shellHeight = 1.0);
	
	// Avec optimisation GPGPU
	void compute(
		GeometricModel* fromGeometry,
		int subdivision,
		float shellHeight);

	GPUVoxelVector(const char* fileName, int SSBO_binding = 4);

	
	void loadDataToGPU(int SSBO_binding);

	GPUBuffer* getAdressOfBuffer();
	void changeVoxelBuffer(GPUBuffer* vBuffer);

	BoundingBoxModelGL* getBoundingBoxModel();

private:
	void createData(int SSBO_binding);

	GLProgramPipeline* computePipeline;
	GLProgram* computeProgram;
	
	GPUBuffer* voxelBuffer;
	GPUBuffer* flagBuffer;
	BoundingBoxModelGL*  GPU_AABB;




};


class OctreeNode
{
public:
	glm::vec3 center;
	OctreeNode* sons[2][2][2];
};

class GPUVoxelOctree : public GPUVoxelVector
{
public:
	GPUVoxelOctree();
	GPUVoxelOctree(
		GeometricModel* fromGeometry,
		int SSBO_binding = 4,
		int subdivision = 8,
		float shellHeight = 1.0);
	
	OctreeNode* rootNode();

private:
	
	

	std::vector<OctreeNode*> nodeStack;
	
	
	

};
