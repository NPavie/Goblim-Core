
#ifdef WIN32
#include "Utils/dirent.h"
#else
#include <dirent.h>
#endif

#include "KernelList.h"
#include <fstream>
#include "Utils/GLError.h"


KernelList::KernelList()
{
	defaultBindingPoint = 3;
	
	renderKernels = true;
	renderGrid = false;
	activeAA = true;
	activeShadows = false;
	modeSlicing = false;
	activeWind = false;
	activeMouse = false;;

	kernelBuffer = new GPUBuffer("kernelBuffer");
	gridHeight = 0.1;


	windFactor = 1.0;
	windSpeed = 1.0;
	mouseFactor = 1.0;
	mouseRadius = 1.0;
}

KernelList::~KernelList()
{

	delete kernelBuffer;
	delete kernelArray;

	delete tex_model_file;
	delete tex_color_maps;
	delete tex_density_maps;
	delete tex_distribution_maps;
	

	TwDeleteAllBars();


}

void KernelList::prepareGPUData()
{
	kernelBufferSize = sizeof(glm::ivec4) + this->size() * sizeof(kernel);
	kernelArray = (KernelBufferData*)malloc(kernelBufferSize);

	

	kernelBuffer->create(kernelBufferSize, GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
	kernelBuffer->setBindingPoint(3);
	// Loading models textures in GPU
	tex_model_file = new GPUTexture2DArray("Kernel_Models", 1024, 1024, str_model_file.size());
	int i = 0;
	for (vector<string>::iterator it_model = str_model_file.begin(); it_model != str_model_file.end(); ++it_model){
		cout << "Loading model file : " << (*it_model) << " in GPU kernel list" << endl;
		tex_model_file->addLayer((*it_model), i, true, glm::vec4(0.0));
		++i;
	}

	// Loading density textures in GPU   
	tex_density_maps = new GPUTexture2DArray("Density_maps", 1024, 1024, str_density_maps.size());
	i = 0;
	for (vector<string>::iterator it_density = str_density_maps.begin(); it_density != str_density_maps.end(); ++it_density){
		cout << "Loading density file : " << (*it_density) << " in GPU kernel list" << endl;
		tex_density_maps->addLayer((*it_density), i, true, glm::vec4(0.0));
		++i;
	}

	// Loading scaling textures in GPU
	tex_scale_maps = new GPUTexture2DArray("Scale_maps", 1024, 1024, str_scale_maps.size());
	i = 0;
	for (vector<string>::iterator it_scale = str_scale_maps.begin(); it_scale != str_scale_maps.end(); ++it_scale){
		cout << "Loading scale file : " << (*it_scale) << " in GPU kernel list" << endl;
		tex_scale_maps->addLayer((*it_scale), i, true, glm::vec4(0.0));
		++i;
	}

	// Loading distribution textures in GPU
	tex_distribution_maps = new GPUTexture2DArray("Distribution_maps", 1024, 1024, str_distribution_maps.size());
	i = 0;
	for (vector<string>::iterator it_distribution = str_distribution_maps.begin(); it_distribution != str_distribution_maps.end(); ++it_distribution){
		cout << "Loading distribution file : " << (*it_distribution) << " in kernel list" << endl;
		tex_distribution_maps->addLayer((*it_distribution), i, true, glm::vec4(0.0));
		++i;
	}

	// Loading Color maps in GPU
	tex_color_maps = new GPUTexture2DArray("Color_maps", 1024, 1024, str_color_maps.size());
	i = 0;
	for (vector<string>::iterator it_color = str_color_maps.begin(); it_color != str_color_maps.end(); ++it_color){
		cout << "Loading distribution file : " << (*it_color) << " in kernel list" << endl;
		tex_color_maps->addLayer((*it_color), i, true, glm::vec4(0.0));
		++i;
	}

	firstUpdate = true;
	buffer_isBound = false;
	density_isBound = false;
	blade_isBound = false;
	scale_isBound = false;
	distribution_isBound = false;
	color_isBound = false;

}

void KernelList::createDefaultKernel(int n /*= 1*/)
{
	for (int i = 0; i < n; ++i)
	{
		this->push_back(new kernel);

		this->back()->data[0] = glm::vec4(0.0, -1.0, -1.0, 0.0);	// inactive and no models load
		this->back()->data[1] = glm::vec4(1.0, 1.0, 1.0, 1.0);		// default space shape
		this->back()->data[2] = glm::vec4(0.0, 0.0, 0.0, 0.0);		// no distribution
		this->back()->data[3] = glm::vec4(-1.0, -1.0, -1.0, -1.0);	// No control maps loaded
		this->back()->data[4] = glm::vec4(0.0, 0.0, 0.0, 0.0);		// No influence
		this->back()->data[5] = glm::vec4(1.0, 1.0, 1.0, 1.0);		// default light settings
		this->back()->data[6] = glm::vec4(1.0, 1.0, 1.0, 1.0);		// default gaussian settings
		this->back()->data[7] = glm::vec4(0.0, 0.0, 0.5, 0.0);		// default distribution min settings
		this->back()->data[8] = glm::vec4(0.5, 0.5, 0.5, 0.5);		// default distribution min settings
		this->back()->data[9] = glm::vec4(0.5, 0.5, 0.5, 1.0);		// default color and transparency
	}
}

void KernelList::loadAnttweakMenu()
{
	for (int i = 0; i < this->size(); i++)
	{
		kernelMenus.push_back(TwNewBar(kernelNames.at(i).c_str()));

		// ModelData : data index = 0 
		TwAddVarRW(kernelMenus[i], "Is Active", TW_TYPE_FLOAT, &this->at(i)->data[0].x, "min=0 max=1 step=1");

		// kernelShape 1 
		TwAddVarRW(kernelMenus[i], "Eval rX", TW_TYPE_FLOAT, &this->at(i)->data[1].x, "min=0 step=0.00001");
		TwAddVarRW(kernelMenus[i], "Eval rY", TW_TYPE_FLOAT, &this->at(i)->data[1].y, "min=0 step=0.00001");
		TwAddVarRW(kernelMenus[i], "Eval rZ", TW_TYPE_FLOAT, &this->at(i)->data[1].z, "min=0 step=0.00001");
		TwAddVarRW(kernelMenus[i], "Eval power", TW_TYPE_FLOAT, &this->at(i)->data[1].w, "min=0 step=0.00001");

		// ModelShape 6
		TwAddVarRW(kernelMenus[i], "Shape rX", TW_TYPE_FLOAT, &this->at(i)->data[6].x, "min=0 step=0.001");
		TwAddVarRW(kernelMenus[i], "Shape rY", TW_TYPE_FLOAT, &this->at(i)->data[6].y, "min=0 step=0.001");
		TwAddVarRW(kernelMenus[i], "Shape rZ", TW_TYPE_FLOAT, &this->at(i)->data[6].z, "min=0 step=0.001");
		TwAddVarRW(kernelMenus[i], "Shape scale", TW_TYPE_FLOAT, &this->at(i)->data[6].w, "min=0 step=0.001");
		
		// Distribution min 7
		TwAddVarRW(kernelMenus[i], "Distrib Min", TW_TYPE_COLOR4F, &this->at(i)->data[7], "min=0 step=0.01");

		// Distribution max 8
		TwAddVarRW(kernelMenus[i], "Distrib Max", TW_TYPE_COLOR4F, &this->at(i)->data[8], "min=0 step=0.01");
		
		// Color 9
		TwAddVarRW(kernelMenus[i], "Color and alpha", TW_TYPE_COLOR4F, &this->at(i)->data[9], "min=0 step=0.01");

		//TwAddVarRW(kernelMenus[i], "Distrib yMin", TW_TYPE_FLOAT, &this->at(i)->data[7].y, "min=0 step=0.01");
		//TwAddVarRW(kernelMenus[i], "Distrib zMin", TW_TYPE_FLOAT, &this->at(i)->data[7].z, "min=0 step=0.01");
		
		

		// distributionData 2 
		TwAddVarRW(kernelMenus[i], "dMax", TW_TYPE_FLOAT, &this->at(i)->data[2].x, "min=0 step=0.1");
		TwAddVarRW(kernelMenus[i], "Grid Subdivision", TW_TYPE_FLOAT, &this->at(i)->data[2].y, "min=1 ");
		TwAddVarRW(kernelMenus[i], "Density Per Cell", TW_TYPE_FLOAT, &this->at(i)->data[2].z, "min=0 ");
		TwAddVarRW(kernelMenus[i], "Distrib Type", TW_TYPE_FLOAT, &this->at(i)->data[2].w, "min=0 step=1");

		

		// ControlMapsIds 3  - Control maintenant automatisé au chargement
		//TwAddVarRW(kernelMenus[i],"Model Texture", TW_TYPE_FLOAT, &this->at(i)->information.y, "min=0 ");
		//TwAddVarRW(kernelMenus[i],"Density Map", TW_TYPE_FLOAT, &this->at(i)->information.z, "min=0 ");
		//TwAddVarRW(kernelMenus[i],"Scale Map", TW_TYPE_FLOAT, &this->at(i)->information.w, "min=0 ");
		//TwAddVarRW(kernelMenus[i],"Distribution map",TW_TYPE_FLOAT,&this->at(i)->distribution.z,"min=0 ");

		//ControlMapsInfluence 4
		TwAddVarRW(kernelMenus[i], "Density Map influence", TW_TYPE_FLOAT, &this->at(i)->data[4].x, "min=0 max=1 step=0.01");
		TwAddVarRW(kernelMenus[i], "Distribution Map Influence", TW_TYPE_FLOAT, &this->at(i)->data[4].y, "min=0 step=0.01 max=1");
		TwAddVarRW(kernelMenus[i], "Scale Map Influence", TW_TYPE_FLOAT, &this->at(i)->data[4].z, "min=0 step=0.01 max=1");
		TwAddVarRW(kernelMenus[i], "Color Map Influence", TW_TYPE_FLOAT, &this->at(i)->data[4].w, "min=0 step=0.01 max=1");

		// lightData 5
		TwAddVarRW(kernelMenus[i], "light attenuation", TW_TYPE_FLOAT, &this->at(i)->data[5].x, "min=0 step=0.01");
		TwAddVarRW(kernelMenus[i], "light emittance", TW_TYPE_FLOAT, &this->at(i)->data[5].y, "min=0 step=0.01");
		TwAddVarRW(kernelMenus[i], "light reflectance", TW_TYPE_FLOAT, &this->at(i)->data[5].z, "min=0 step=0.01");
		TwAddVarRW(kernelMenus[i], "Volumetric obscurance factor", TW_TYPE_FLOAT, &this->at(i)->data[5].w, "min=0 step=0.01");



	}
	if (this->size() > 0)
	{
		kernelListMenu = TwNewBar("All Kernels");
		TwAddVarRW(kernelListMenu, "Grid height", TW_TYPE_FLOAT, &gridHeight, "min=0 step=0.001");
		TwAddVarRW(kernelListMenu, "Active Kernel", TW_TYPE_BOOL8, &renderKernels, "");
		TwAddVarRW(kernelListMenu, "Active Grid", TW_TYPE_BOOL8, &renderGrid, "");
		TwAddVarRW(kernelListMenu, "Active Shadows", TW_TYPE_BOOL8, &activeShadows, "");
		TwAddVarRW(kernelListMenu, "Active AA", TW_TYPE_BOOL8, &activeAA, "");
		
		TwAddVarRW(kernelListMenu, "Active wind effect", TW_TYPE_BOOL8, &activeWind, "");
		TwAddVarRW(kernelListMenu, "Wind factor", TW_TYPE_FLOAT, &windFactor, "min=0 step=0.01");
		TwAddVarRW(kernelListMenu, "Wind speed", TW_TYPE_FLOAT, &windSpeed, "min=0 step=0.01");

		TwAddVarRW(kernelListMenu, "Active Mouse interaction", TW_TYPE_BOOL8, &activeMouse, "");
		TwAddVarRW(kernelListMenu, "Mouse factor", TW_TYPE_FLOAT, &mouseFactor, "min=0 step=0.01");
		TwAddVarRW(kernelListMenu, "Mouse radius", TW_TYPE_FLOAT, &mouseRadius, "min=0 step=0.01");


		TwAddVarRW(kernelListMenu, "Slicing Mode", TW_TYPE_BOOL8, &modeSlicing, "");
		
	}
}

void KernelList::updateGPUData()
{

	kernelArray->nbKernel = glm::ivec4(this->size(), 0, 0, 0);
	for (int i = 0; i < this->size(); i++)
	{
		kernelArray->kernels[i] = *(this->at(i));
	}

	// Update the shader storage buffer
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, kernelBuffer->getBuffer());
	
	KernelBufferData* ptr = (KernelBufferData*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
	if (ptr != NULL)
	{
		memcpy(ptr, kernelArray, kernelBufferSize);
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	}
	else std::cout << "Error updating KernelList \n";

	//glm::ivec4 nbKernel = glm::vec4(0, 0, 0, 0);
	//nbKernel.x = (int)this->size();
	//glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(glm::ivec4), &nbKernel);
	//
	////int offset = sizeof(int);
	//int offset = sizeof(glm::ivec4);
	//int kernelSize = sizeof(kernel);
	//kernel* ptr = (kernel*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, kernelSize*nbKernel.x, GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT);
	//if (ptr != NULL)
	//{
	//	int i = 0;
	//	for (vector<kernel*>::iterator it = this->begin(); it != this->end(); ++it){
	//		for (int j = 0; j < 7; j++)
	//		{
	//			ptr[i].data[j] = (*it)->data[j];
	//		}
	//		++i;
	//	}
	//	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	//}
	//else std::cout << "Error updating KernelList \n";
	
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	kernelBuffer->bind();
	

}

void KernelList::displayGPUData()
{


	glBindBuffer(GL_SHADER_STORAGE_BUFFER, kernelBuffer->getBuffer());
	int nbKernel = 0;
	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLint), &nbKernel);

	int offset = sizeof(glm::vec4); // obliger de mettre un offset a n * vec4
	int kernelSize = sizeof(kernel);
	kernel* ptr = (kernel*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, offset, kernelSize*nbKernel, GL_MAP_READ_BIT);
	if (ptr != NULL)
	{
		for (int i = 0; i < nbKernel; i++){
			displayKernel(&ptr[i]);
		}
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	} // 
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	kernelBuffer->bind();

}

void KernelList::bindBuffer(int boundingPoint /*= 0*/)
{
	if (boundingPoint < 0) kernelBuffer->bind();
	else if (defaultBindingPoint < 0)
	{
		kernelBuffer->setBindingPoint(boundingPoint);
		kernelBuffer->bind();
		
	}
	
	buffer_isBound = true;
}

void KernelList::releaseTextures()
{

	if (density_isBound)
	{
		tex_density_maps->release();
		density_isBound = false;
	}

	if (blade_isBound)
	{
		tex_model_file->release();
		blade_isBound = false;
	}

	if (scale_isBound)
	{
		tex_scale_maps->release();
		scale_isBound = false;
	}

	if (distribution_isBound)
	{
		tex_distribution_maps->release();
		distribution_isBound = false;
	}
	if (color_isBound)
	{
		tex_color_maps->release();
		color_isBound = false;
	}
}

void KernelList::releaseBuffer()
{
	if (buffer_isBound)
	{
		kernelBuffer->release();
		buffer_isBound = false;
	}
}

void KernelList::createKernel(
	glm::vec4 & ModelData,
	glm::vec4 & ModelShape,
	glm::vec4 & kernelShape,
	glm::vec4 & distributionData,
	glm::vec4 & ControlMapsIds,
	glm::vec4 & ControlMapsInfluence,
	glm::vec4 & lightData
	)
{
	this->push_back(new kernel);
	this->back()->data[0] = ModelData;
	this->back()->data[1] = kernelShape;
	this->back()->data[2] = distributionData;
	this->back()->data[3] = ControlMapsIds;
	this->back()->data[4] = ControlMapsInfluence;
	this->back()->data[5] = lightData;
	this->back()->data[6] = ModelShape;

}

void KernelList::addDensityMap(std::string densityMapFile)
{
	str_density_maps.push_back(densityMapFile);
}

void KernelList::addModel(std::string modelTextureFile)
{
	str_model_file.push_back(modelTextureFile);
}

void KernelList::addScaleMap(std::string scaleMapFile)
{
	str_scale_maps.push_back(scaleMapFile);
}

void KernelList::addDistributionMap(std::string distributionMapFile)
{
	str_distribution_maps.push_back(distributionMapFile);
}

void KernelList::addColorMap(std::string colorMapFile)
{
	str_color_maps.push_back(colorMapFile);
}

void KernelList::bindDensityMaps(int channel /*= 0*/)
{
	tex_density_maps->bind(channel);
	density_isBound = true;
}

void KernelList::bindModels(int channel /*= 0*/)
{
	tex_model_file->bind(channel);
	blade_isBound = true;
}

void KernelList::bindScaleMaps(int channel /*= 0*/)
{
	tex_scale_maps->bind(channel);
	scale_isBound = true;
}

void KernelList::bindDistributionMaps(int channel /*= 0*/)
{
	tex_distribution_maps->bind(channel);
	distribution_isBound = true;
}

void KernelList::bindColorMaps(int channel /*= 0*/)
{
	tex_color_maps->bind(channel);
	color_isBound = true;
}

void KernelList::loadFromFolder(std::string path, std::string kernelName)
{
	DIR *dir;
	struct dirent *ent;
	dir = opendir(path.c_str());
	bool isFinalFolder = true;
	bool isOpenable = false;
	if (dir != NULL)
	{
		isOpenable = true;
		while ((ent = readdir(dir)) != NULL)
		{
			if (strchr(ent->d_name, '.') == NULL)
			{
				loadFromFolder(path + "/" + ent->d_name, kernelName + ent->d_name);
				isFinalFolder = false;
			}


		}
		closedir(dir);
	}

	if (isOpenable && isFinalFolder)
	{
		int model_texID_begin = str_model_file.size();
		int nbModelTextures = 0;
		dir = opendir(path.c_str());

		std::string::size_type pos = path.find("//");
		std::string folder = path.substr(pos + 2);
		kernelNames.push_back(folder);

		int densityIndex = -1;
		int modelIndex = -1;
		int scaleIndex = -1;
		kernel* toAdd = new kernel; // default
		toAdd->data[0] = glm::vec4(0.0, model_texID_begin, -1.0, 0.0);
		toAdd->data[1] = glm::vec4(1.0, 1.0, 1.0, 0.0);		// default shape
		toAdd->data[2] = glm::vec4(0.0, 0.0, 0.0, 0.0);		// no distribution
		toAdd->data[3] = glm::vec4(-1.0, -1.0, -1.0, -1.0);	// No control maps loaded
		toAdd->data[4] = glm::vec4(0.0, 0.0, 0.0, 0.0);		// No influence
		toAdd->data[5] = glm::vec4(1.0, 1.0, 1.0, 1.0);		// default light settings
		toAdd->data[6] = glm::vec4(1.0, 1.0, 1.0, 1.0);		// default shape
		toAdd->data[7] = glm::vec4(0.0, 0.0, 0.5, 0.0);		// default distrib	
		toAdd->data[8] = glm::vec4(0.5, 0.5, 0.5, 0.5);		// default distrib
		toAdd->data[9] = glm::vec4(0.5, 0.5, 0.5, 1.0);		// default distrib
		while ((ent = readdir(dir)) != NULL)
		{
			std::string fileName(ent->d_name);
			if (fileName.compare(0, 5, "model") == 0)
			{
				toAdd->data[0].z = str_model_file.size();
				this->addModel(path + "/" + ent->d_name);
			}
			else if (fileName.compare("density.png") == 0)
			{
				toAdd->data[3].x = str_density_maps.size();
				this->addDensityMap(path + "/" + ent->d_name);
			}
			else if (fileName.compare("distribution.png") == 0)
			{
				toAdd->data[3].y = str_distribution_maps.size();
				this->addDistributionMap(path + "/" + ent->d_name);
			}
			else if (fileName.compare("scale.png") == 0)
			{
				toAdd->data[3].z = str_scale_maps.size();
				this->addScaleMap(path + "/" + ent->d_name);
			}
			else if (fileName.compare("color.png") == 0)
			{
				toAdd->data[3].w = str_color_maps.size();
				this->addColorMap(path + "/" + ent->d_name);
			}
			else if (fileName.compare("config.txt") == 0)
			{
				str_config_files.push_back((path + "/" + ent->d_name)); // Storing the file name for saving modification
				FILE* config = fopen((path + "/" + ent->d_name).c_str(), "r");
				if (config != NULL)
				{
					fscanf(config,
						"kernel : %f %f %f %f \n shape : %f %f %f %f \n distribution : %f %f %f %f \n control : %f %f %f %f \n light : %f %f %f %f",
						&(toAdd->data[1].x), &(toAdd->data[1].y), &(toAdd->data[1].z), &(toAdd->data[1].w),
						&(toAdd->data[6].x), &(toAdd->data[6].y), &(toAdd->data[6].z), &(toAdd->data[6].w),
						&(toAdd->data[2].x), &(toAdd->data[2].y), &(toAdd->data[2].z), &(toAdd->data[2].w),
						&(toAdd->data[4].x), &(toAdd->data[4].y), &(toAdd->data[4].z), &(toAdd->data[4].w),
						&(toAdd->data[5].x), &(toAdd->data[5].y), &(toAdd->data[5].z), &(toAdd->data[5].w)
						);

					fclose(config);
				}
				else
				{
					std::cout << "Error in opening the config file of " << kernelName << endl;
				}

			}
			isFinalFolder = false;



		}
		closedir(dir);
		this->push_back(toAdd);
		cout << "Loading new kernel : " << kernelName << endl;
		displayKernel(toAdd);
	}
}


void displayKernel(kernel* toDisplay)
{
	// 0 -- ModelData :  isActive, model_texIDs_begin, model_texIDs_end
	// 1 -- kernelShape : rx, ry, scale, curvature (pour plus tard)
	// 2 -- distributionData : distribution_ID(pour distribution différentes par noyau), subdivision, densityPerCell, 
	// 3 -- ControlMapsIds : density_texID, distribution_texID, scale_texID, colorMap_texId
	// 4 -- ControlMapsInfluence : densityMap_influence, distributionMap_influence, scaleMap_influence, colorMap_influence
	// 5 -- lightData : attenuation, emittance, reflectance, volumetric obscurance factor
	// TODO 6 - Model Shape
	cout << "ModelData : " << toDisplay->data[0].x << " ; " << toDisplay->data[0].y << " ; " << toDisplay->data[0].z << " ; " << toDisplay->data[0].w << endl;
	cout << "kernelShape : " << toDisplay->data[1].x << " ; " << toDisplay->data[1].y << " ; " << toDisplay->data[1].z << " ; " << toDisplay->data[1].w << endl;
	cout << "distributionData : " << toDisplay->data[2].x << " ; " << toDisplay->data[2].y << " ; " << toDisplay->data[2].z << " ; " << toDisplay->data[2].w << endl;
	cout << "ControlMapsIds : " << toDisplay->data[3].x << " ; " << toDisplay->data[3].y << " ; " << toDisplay->data[3].z << " ; " << toDisplay->data[3].w << endl;
	cout << "ControlMapsInfluence : " << toDisplay->data[4].x << " ; " << toDisplay->data[4].y << " ; " << toDisplay->data[4].z << " ; " << toDisplay->data[4].w << endl;
	cout << "lightData : " << toDisplay->data[5].x << " ; " << toDisplay->data[5].y << " ; " << toDisplay->data[5].z << " ; " << toDisplay->data[5].w << endl;

}

float KernelList::getGridHeight()
{
	return this->gridHeight;
}


void KernelList::setGridHeight(float newGridHeight)
{
	this->gridHeight = newGridHeight;
}

GPUBuffer* KernelList::getBuffer()
{
	return kernelBuffer;
}

void KernelList::mapToProgramBlock(GLProgram* stage, string blockName)
{
	GLint blockIndex = glGetProgramResourceIndex(stage->getProgram(), GL_SHADER_STORAGE_BLOCK, blockName.c_str());
	if (blockIndex != GL_INVALID_INDEX)
	{
		glShaderStorageBlockBinding(stage->getProgram(), blockIndex, defaultBindingPoint);
	}
}

TwBar*	KernelList::getKernelListMenu()
{
	return kernelListMenu;
}