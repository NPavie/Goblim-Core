#pragma once
#include "GPUResources/Buffers/GPUBuffer.h"

#include <vector>

template <class GPUDataType>
class GPUArray
{
public:
	GPUArray();
	GPUArray(std::vector<GPUDataType> CPUArray);
	~GPUArray();

	void fillFromCPUArray(std::vector<GPUDataType> CPUArray);

	GPUBuffer* createGPUBuffer(std::string name, unsigned int bindingPoint);

	void updateGPUBuffer(GPUBuffer* buffer);

	int size;
	/** Array of GPU data */
	GPUDataType* dataArray;

};

template < class GPUDataType >
GPUArray<GPUDataType>::GPUArray()
{
	dataArray = NULL;
	this->size = 0;
}

template < class GPUDataType >
GPUArray<GPUDataType>::~GPUArray()
{
	if (dataArray != NULL)	delete[] dataArray;
}

template < class GPUDataType >
GPUArray<GPUDataType>::GPUArray(std::vector<GPUDataType> CPUArray)
{
	// transfert vector data to aligned arrays
	int bufferMemorySize = sizeof(int) + CPUArray.size() * sizeof(GPUDataType);
	this->size = CPUArray.size();
	this->dataArray = (GPUDataType*)malloc(CPUArray.size() * sizeof(GPUDataType));

	fillFromCPUArray(CPUArray);

}

template < class GPUDataType >
void GPUArray<GPUDataType>::fillFromCPUArray(std::vector<GPUDataType> CPUArray)
{
	for (int i = 0; i < CPUArray.size(); ++i)
	{
		this->dataArray[i] = CPUArray[i];
	}
}


template < class GPUDataType >
GPUBuffer* GPUArray<GPUDataType>::createGPUBuffer(std::string name, unsigned int bindingPoint)
{
	// Creating a new GPUbuffer according the underlying data
	int arrayMemorySize = this->size * sizeof(GPUDataType);
	GPUBuffer* out = new GPUBuffer(name);
	out->create(arrayMemorySize + sizeof(glm::vec4), GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
	out->setBindingPoint(bindingPoint);
	updateGPUBuffer(out);
	return out;
}

template < class GPUDataType >
void GPUArray<GPUDataType>::updateGPUBuffer(GPUBuffer* buffer)
{
	int arrayMemorySize = this->size * sizeof(GPUDataType);
	// Fill the memory with the corresponding data
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer->getBuffer());
	GPUDataType* ptr = (GPUDataType*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	if (ptr != NULL) {
		memcpy(ptr, dataArray, arrayMemorySize);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
	else std::cout << "Error updating buffer " << std::endl;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}
