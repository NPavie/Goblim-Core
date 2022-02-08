
#ifndef _GOBLIM_IMAGE_READER_
#define _GOBLIM_IMAGE_READER_
#include <string>
#include <gfl/libgfl.h>

class GFLImageReader
{
public:
	GFLImageReader();
	~GFLImageReader();
	
GFL_BITMAP*  loadImage(std::string filename);
unsigned int load2DTexture(std::string filename, int width=-1,int height=-1);
private:
	static bool init;
	GFL_LOAD_PARAMS* params;
};


#endif
