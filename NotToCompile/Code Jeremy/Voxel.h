#pragma once

#include <glm\glm.hpp>
#include <vector>
#include "GeometricModel.h"
#include "BoundingBox\BoundingBoxModelGL.h"
#include "OpenGL4\GLProgramPipeline.h"

// structure de donnée d'un voxel
struct VoxelData
{
	glm::vec4 position;
	glm::vec4 normale;
	glm::vec4 tangente;
	glm::vec4 coord_texture;
	glm::vec4 coord_surface;// UVW de l'élément de surface associé + distance à cet élément de surface
	glm::ivec4 boolean;
};

struct geometry
{
	glm::vec4 point;
	glm::vec4 normale;
	glm::vec4 tangente;
	glm::vec4 coord;
	glm::vec4 pointextr;
};
struct Cube
{
	glm::vec4 minCoord;
	glm::vec4 maxCoord;
	glm::vec4 halfVector;
	glm::ivec4 nbVoxelPerAxis;
};
struct Triangle
{
	glm::ivec4 s;
	glm::vec4 minPoint;
	glm::vec4 maxPoint;
	glm::ivec4 offsetVoxel;
};
struct geometryArray
{
	glm::ivec4 size;
	Cube cube;
	Triangle triangles[];
};

struct deleteArray
{
	glm::ivec4 del;
};

class Voxel
{
public:
	Voxel();
	Voxel(VoxelData & toCopy);
	Voxel(
		glm::vec4 & position, 
		glm::vec4 & normale, 
		glm::vec4 & tangente, 
		glm::vec4 & coord_texture, 
		glm::vec4 & coord_surface);
	
	~Voxel();

	VoxelData*	getAdressOfData();
	VoxelData	getCopyOfData();

	void changePosition(glm::vec4 & position);
	void changeNormale(glm::vec4 & normale);
	void changeTangente(glm::vec4 & tangente);
	void changeCoord_texture(glm::vec4 & coord_texture);
	void changeCoord_surface(glm::vec4 & coord_surface);
	VoxelData getVoxelData()
	{
		return data;
	}

protected:
	VoxelData data;
};


class VoxelVector : public std::vector<Voxel>
{
public:
	VoxelVector();
	VoxelVector::VoxelVector(int subdivision = 8);
	VoxelVector(
		GeometricModel* fromGeometry, 
		int subdivision = 8, 
		float shellHeight = 1.0);
	

	void compute(
		GeometricModel* fromGeometry, 
		int subdivision, 
		float shellHeight);

	// Return a ptr to GeometricBoundingBox
	GeometricBoundingBox* getGeometricBoundingBox();

	float voxelHalfSize;
	glm::vec3 minCoord;
	glm::vec3 maxCoord;

protected:
	bool computeDone;

	std::vector<glm::vec3> listExtrudedVertex;
	GeometricBoundingBox* AABB;

	

};

// GPU structure
struct VoxelArray
{
	glm::ivec4 size;
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

	void loadDataToGPU(int SSBO_binding);

	GPUBuffer* getAdressOfBuffer();
	void changeVoxelBuffer(GPUBuffer* vBuffer);

	BoundingBoxModelGL* getBoundingBoxModel();

private:
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
