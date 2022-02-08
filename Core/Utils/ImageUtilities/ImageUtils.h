#ifndef _IMAGE_UTILS
#define _IMAGE_UTILS
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <SOIL/SOIL.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string.h>

int	up_scale_image	(const unsigned char* const orig,int width, int height, int channels,unsigned char* resampled,int resampled_width, int resampled_height	);
unsigned char* resize(unsigned char* orig,int size,int channels,int resampled_size);
GLint load2DTexture(std::string filename,unsigned int flags);

unsigned int loadLayer	(const unsigned char *const data,int width, int height, int channels,int layer,int target_width,int target_height,unsigned int flags
	);

unsigned int loadSimpleLayer(const unsigned char *const data,int width, int height, int channels,unsigned int flags,
		int layer,
		int target_size
	);


/*
	Jonathan Dummer
	2007-07-31-10.32

	simple DXT compression / decompression code

	public domain
*/
/**

	This function downscales an image.
	Used for creating MIPmaps,
	the incoming image should be a
	power-of-two sized.
**/
int
	mipmap_image
	(
		const unsigned char* const orig,
		int width, int height, int channels,
		unsigned char* resampled,
		int block_size_x, int block_size_y
	);
#endif