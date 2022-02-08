#ifndef _GPUTextureCubeMap_H
#define _GPUTextureCubeMap_H

#include "GPUTexture.h"
#include <string>

class GPUTextureCubeMap : public GPUTexture
{
public :

	GPUTextureCubeMap(std::string name);
	~GPUTextureCubeMap();
	
	bool create(int width, int height);


	bool create(std::string file_right,std::string file_left,std::string file_top,std::string file_bottom,std::string file_back,std::string file_front);
	
	void setUpSampler(GLint wrap,GLint filter);

	
};

#endif // !_GPUCubeMap_H
