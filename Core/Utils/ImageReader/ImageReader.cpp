
#include "Utils/ImageReader/ImageReader.h"
#include <GL/glew.h>
#include <iostream>

bool GFLImageReader::init = false;

GFLImageReader::GFLImageReader()
{
	if (!init)
	{
		gflLibraryInit();
		
		//params->ColorModel = GFL_RGBA 	;
	}

}
GFLImageReader::~GFLImageReader()
{
}


GFL_BITMAP*  GFLImageReader::loadImage(std::string filename)
{
	
	GFL_LOAD_PARAMS params ;
		gflGetDefaultLoadParams(&params);
	
	GFL_BITMAP* bitmap = NULL;
	
	GFL_FILE_INFORMATION info ;
	
	GFL_ERROR err = gflLoadBitmap(filename.c_str(),  &bitmap, &params, &info);
		
	if (err != GFL_NO_ERROR)
	{
		std::cout << "GFL ERROR : " << filename << " : " <<gflGetErrorString(err) << std::endl;
		
		return NULL;
	}
	
	return bitmap;

}
unsigned int  GFLImageReader::load2DTexture(std::string filename, int twidth,int theight)
{
	unsigned int tex;
	GFL_BITMAP* bitmap = loadImage(filename);

	if (bitmap == NULL)
			return -1;

	int width,height;
	width = bitmap->Width;
	height = bitmap->Height;

	GFL_ERROR err = gflChangeColorDepth(bitmap,NULL,GFL_MODE_TO_RGBA,GFL_MODE_ADAPTIVE);
	
	if (err != GFL_NO_ERROR)
	{
		std::cout << "GFL ERROR : changing color depth for " << filename << " : " <<gflGetErrorString(err) << std::endl;
		return -1;
	}
	
	if (twidth != -1 || theight != -1)
	{
		GFL_ERROR err = gflResize(bitmap,NULL,twidth,theight,GFL_RESIZE_BILINEAR,0);
		width = twidth;
		height = theight;
		
		if (err != GFL_NO_ERROR)
		{
			std::cout << "GFL ERROR : resizing " << filename << " : " <<gflGetErrorString(err) << std::endl;
			return -1;
		}
	}

	
	glGenTextures(1,&tex);
	glBindTexture(GL_TEXTURE_2D,tex);
	glTexStorage2D(GL_TEXTURE_2D,10,GL_RGBA8,width,height);
	glTexSubImage2D(GL_TEXTURE_2D,0,0,0,width,height,GL_RGBA,GL_UNSIGNED_BYTE,bitmap->Data);

	gflFreeBitmap(bitmap);


	return tex;

}