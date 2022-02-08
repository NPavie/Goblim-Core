#ifndef _KERNEL_STRUCTURE_H
#define _KERNEL_STRUCTURE_H

#include <glm/glm.hpp>
#include <vector>
#include <sstream>
#include <AntTweakBar/AntTweakBar.h>

#include "GPUResources/Buffers/GPUBuffer.h"
#include "GPUResources/Buffers/GPUShaderStorageBuffer.h"
#include "GPUResources/Textures/GPUTexture2DArray.h"
#include "Engine/OpenGL/Managers/GPUVariable.hpp"

#include "Engine/OpenGL/v4/GLProgram.h"




// kernel structure for details rendering
struct kernel{
	// Changement d'agencement pour un tableau de vec4: 
	glm::vec4 data[10];
	// Index of data :
	// 0 -- ModelData :  isActive, model_texIDs_begin, model_texIDs_end
	// 1 -- kernelShape : rx, ry, rz, power
	// 2 -- distributionData : dMax, subdivision, densityPerCell, distribID
	// 3 -- ControlMapsIds : density_texID, distribution_texID, scale_texID, colorMap_texId
	// 4 -- ControlMapsInfluence : densityMap_influence, distributionMap_influence, scaleMap_influence, colorMap_influence
	// 5 -- lightData : attenuation, emittance, reflectance, volumetric obscurance factor
	// 6 -- ModelShape : rx , ry , rz, scale
	// 7 -- distrib_min : x,y,z
	// 8 -- distrib_max : x,y,z
	// 9 -- Color and transparency

};

struct KernelBufferData
{
	glm::ivec4 nbKernel; //.x
	kernel kernels[];
};

void displayKernel(kernel* toDisplay);

// class to handle the kernel list + GPU transfer
class KernelList : public std::vector<kernel*>
{
public :
	
	KernelList();
	~KernelList();

	/*
	@brief Create the buffer in the GPU Memory
	*/
	void prepareGPUData();
	
	/*
	@brief  add n new default kernel inside the list
	@param	n	number of kernels to create
	*/
	void createDefaultKernel(int n = 1);
	
	/*
	@brief  Create a new kernel with defined settings inside the list
	@param	ModelData				isActive, model_texIDs_begin, model_texIDs_end
	@param	kernelShape				rx, ry, curvature (pour plus tard), scale
	@param	distributionData		subdivision, densityPerCell, distribution_ID(pour distribution différentes par noyau)
	@param	ControlMapsIds			density_texID, distribution_texID, scale_texID, colorMap_texId
	@param	ControlMapsInfluence	densityMap_influence, distributionMap_influence, scaleMap_influence, colorMap_influence
	@param	lightData				attenuation, emittance, reflectance, volumetric obscurance factor
	*/
	void createKernel(
		glm::vec4& ModelData,
		glm::vec4& ModelShape,
		glm::vec4& kernelShape,
		glm::vec4& distributionData,
		glm::vec4& ControlMapsIds,
		glm::vec4& ControlMapsInfluence,
		glm::vec4& lightData
					);

	/*
	@brief Function to display the AntTweak menu of each kernel in the context
	*/
	void loadAnttweakMenu();

	/*
	@brief Update the GPU memory
	*/
	void updateGPUData();

	void displayGPUData();
	
	/*
	@brief Bind the buffer to use
	@param bindingPoint	GPU binding point of the buffer
	*/
	void bindBuffer(int boundingPoint = -1);

	/*
	-> Release the textures
	*/
	void releaseTextures();

	/*
	@brief Release the buffer of GPU
	*/
	void releaseBuffer();

	/*
	@brief Add a density texture in the list of density maps available
	*/
	void addDensityMap(std::string densityMapFile);

	/*
	-> Add a model texture in the list of models available
	*/
	void addModel(std::string modelTextureFile);

	/**
	*	@brief add a scale map in the kernel resources
	*/
	void addScaleMap(std::string scaleMapFile);

	/**
	*	@brief add a scale map in the kernel resources
	*/
	void addDistributionMap(std::string distributionMapFile);

	/**
	*	@brief add a scale map in the kernel resources
	*/
	void addColorMap(std::string colorMapFile);

	/*
	Bind the Texture2DArray of density maps on GPU on a texture channel (suggestion : set location with a sampler and bind to the sampler value)
	*/
	void bindDensityMaps(int channel = 0);

	/*
	Bind the Texture2DArray of model textures on GPU (suggestion : set location with a sampler and bind to the sampler value)
	*/
	void bindModels(int channel = 0);

	/*
	Bind the Texture2DArray of model textures on GPU (suggestion : set location with a sampler and bind to the sampler value)
	*/
	void bindScaleMaps(int channel = 0);

	/*
	Bind the Texture2DArray of distribution textures on GPU (suggestion : set location with a sampler and bind to the sampler value)
	*/
	void bindDistributionMaps(int channel = 0);

	/*
	Bind the Texture2DArray of distribution textures on GPU (suggestion : set location with a sampler and bind to the sampler value)
	*/
	void bindColorMaps(int channel = 0);

	/*
		Map the GPU data to the program block
	*/
	void mapToProgramBlock(GLProgram* stage, string blockName);

	/*
		Load a kernel configuration list from a path
	*/
	void loadFromFolder(std::string path, std::string kernelName = "");
	void saveConfiguration();

	float getGridHeight();
	void setGridHeight(float newGridHeight);
	TwBar*	getKernelListMenu();

	GPUBuffer* getBuffer();


	float windFactor;
	float windSpeed;

	float mouseFactor;
	float mouseRadius;

	bool renderKernels;
	bool renderGrid;
	bool activeShadows;
	bool activeAA;
	bool modeSlicing;
	bool activeWind;
	bool activeMouse;

	

private :

	size_t kernelBufferSize;
	GPUBuffer*	kernelBuffer;
	KernelBufferData* kernelArray;

	std::vector<std::string> kernelNames;
	bool		buffer_isBound;
	int			defaultBindingPoint;

	std::vector<TwBar*> kernelMenus;
	TwBar*				kernelListMenu;
	
	// Global parameters
	float gridHeight;

	// Textures arrays
	// -- model : blade textures to apply over evaluated kernels
	vector<std::string>	str_model_file;
	GPUTexture2DArray*	tex_model_file;
	bool blade_isBound;

	// -- Density maps
	vector<std::string>	str_density_maps;
	GPUTexture2DArray*	tex_density_maps;
	bool density_isBound;

	// -- Distribution (normal) maps
	vector<std::string> str_distribution_maps;
	GPUTexture2DArray*	tex_distribution_maps;
	bool distribution_isBound;

	// -- Scale maps
	vector<std::string> str_scale_maps;
	GPUTexture2DArray*	tex_scale_maps;
	bool scale_isBound;

	// -- Color maps
	vector<std::string> str_color_maps;
	GPUTexture2DArray*	tex_color_maps;
	bool color_isBound;

	vector<std::string> str_config_files;

	bool firstUpdate;
	
};

#endif // !_KERNEL_STRUCTURE_H
