#include "GPUTexImage.h"

GPUTexImage::GPUTexImage(std::string name) : 
Resource(name)
{
	HUD = new DisplayImage("HUD Image");
	bound_on_channel = 0;
	create();
}

GPUTexImage::~GPUTexImage()
{
	destroy();
	delete HUD;	
	loadSampler();
}

bool GPUTexImage::create()
{
	glGenTextures(1,&m_ImageId);
	return true;
}
void GPUTexImage::destroy()
{
	glDeleteTextures(1,&m_ImageId);
	glDeleteSamplers(1,&m_SamplerId);
}
void GPUTexImage::loadSampler()
{
	glGenSamplers(1 , &m_SamplerId); 	
}

void GPUTexImage::bind(GLuint channel,GLenum accessmode,int level,GLenum format)
{
	bound_on_channel = channel;
	glBindImageTexture(channel,m_ImageId,level,GL_FALSE,0,accessmode,format == 0 ? m_Format:format);
}

void GPUTexImage::release()
{
	glBindImageTexture(bound_on_channel,0,0,GL_FALSE,0,GL_READ_ONLY,m_Format);
}


void GPUTexImage::clear(const glm::vec4 & color)
{

}
void GPUTexImage::display(const glm::vec4 & box)
{
	bind(0);
	HUD->display(box);
	release();
}
void GPUTexImage::fill(void* data)
{
}


std::string GPUTexImage::formatToString(int format)
{
	if (format == -1)
		format = m_Format;

	//GL_RGBA32F rgba32f
//GL_RGBA16F rgba16f
//GL_RG32F rg32f
//GL_RG16F rg16f
//GL_R11F_G11F_B10F r11f_g11f_b10f
//GL_R32F r32f
//GL_R16F r16f
//GL_RGBA32UI rgba32ui
// GL_RGBA16UI rgba16ui
// GL_RGB10_A2UI rgb10_a2ui
// GL_RGBA8UI rgba8ui 
// GL_RG32UI rg32ui 
		
	if (format == GL_RGBA32F)
		return(std::string("rgba32f"));
	if (format == GL_RGBA16F)
		return(std::string("rgba16f"));
		if (format == GL_RG32F)
		return(std::string("rg32f"));
	if (format == GL_RG16F)
		return(std::string("rg16f"));
	if (format == GL_R11F_G11F_B10F)
		return(std::string("r11f_g11f_b10f"));
		if (format == GL_R32F)
		return(std::string("r32f"));
	if (format == GL_R16F)
		return(std::string("r16f"));
	if (format == GL_RGBA32UI)
		return(std::string("rgba32ui"));
		if (format == GL_RGBA16UI)
		return(std::string("rgba16ui"));
	if (format == GL_RGB10_A2UI)
		return(std::string("rgb10_a2ui"));
		if (format == GL_RGBA8UI)
		return(std::string("rgba8ui"));
	if (format == GL_RG32UI)
		return(std::string("rg32ui"));


	// GL_RG16UI rg16ui 
// GL_RG8UI rg8ui 
// GL_R32UI r32ui 
// GL_R16UI r16ui 
// GL_R8UI r8ui 
// GL_RGBA32I rgba32i 
// GL_RGBA16I rgba16i 
// GL_RGBA8I rgba8i 
// GL_RG32I rg32i 
// GL_RG16I rg16i 


		if (format == GL_RG16UI)
		return(std::string("rg16ui"));
	if (format == GL_RG8UI)
		return(std::string("rg8ui"));
		if (format == GL_R32UI)
		return(std::string("r32ui"));
	if (format == GL_R16UI)
		return(std::string("r16ui"));
	if (format == GL_R8UI)
		return(std::string("r8ui"));

	if (format == GL_RGBA32I)
		return(std::string("rgba32i"));
	if (format == GL_RGBA16I)
		return(std::string("rgba16i"));
	if (format == GL_RGBA8I)
		return(std::string("rgba8i"));
		if (format == GL_RG32I)
		return(std::string("rg32i"));
	if (format == GL_RG16I)
		return(std::string("rg16i"));

	// GL_RG8I rg8i 
// GL_R32I r32i 
// GL_R16I r16i 
// GL_R8I r8i 
// GL_RGBA16 rgba16
// GL_RGB10_A2 rgb10_a2 
// GL_RGBA8 rgba8 
// GL_RG16 rg16 
// GL_RG8 rg8 
// GL_R16 r16 
// GL_R8 r8 




		if (format == GL_RG8I)
		return(std::string("rg8i"));
	if (format == GL_R32I)
		return(std::string("r32i"));
		if (format == GL_R16I)
		return(std::string("r16i"));	
	if (format == GL_R8I)
		return(std::string("r8i"));
	if (format == GL_RGBA16)
		return(std::string("rgba16"));
	if (format == GL_RGB10_A2)
		return(std::string("rgb10_a2"));
	if (format == GL_RGBA8)
		return(std::string("rgba8"));
		if (format == GL_RG16)
		return(std::string("rg16"));
	if (format == GL_RG8)
		return(std::string("rg8"));
		if (format == GL_R16)
		return(std::string("r16"));
	if (format == GL_R8)
		return(std::string("r8"));

 // GL_RGBA16_SNORM rgba16_snorm 
// GL_RGBA8_SNORM rgba8_snorm 
// GL_RG16_SNORM rg16_snorm 
// GL_RG8_SNORM rg8_snorm 
// GL_R16_SNORM r16_snorm 
// GL_R8_SNORM r8_snorm

	if (format == GL_RGBA16_SNORM)
		return(std::string("rgba16_snorm"));
	if (format == GL_RGBA8_SNORM)
		return(std::string("rgba8_snorm"));
	if (format == GL_RG16_SNORM)
		return(std::string("rg16_snorm"));
		if (format == GL_RG8_SNORM)
		return(std::string("rg8_snorm"));
	if (format == GL_R16_SNORM)
		return(std::string("r16_snorm"));
	if (format == GL_R8_SNORM)
		return(std::string("r8_snorm"));
		



	 //Internal format  Equivalence      R        G         B         A
 //       ---------------  -----------   -------  -------   -------   -------
 //       RGBA32F             4x32       0[31:0]  1[31:0]   2[31:0]   3[31:0]
 //       RGBA16F             2x32       0[15:0]  0[31:16]  1[15:0]   1[31:16]
 //       RG32F               2x32       0[31:0]  1[31:0] 
 //       RG16F               1x32       0[15:0]  0[31:16] 
 //       R11F_G11F_B10F      1x32       0[10:0]  0[21:11]  0[31:22] 
 //       R32F                1x32       0[31:0]  
 //       R16F                1x16       0[15:0] 

 //       RGBA32UI            4x32       0[31:0]  1[31:0]   2[31:0]   3[31:0]
 //       RGBA16UI            2x32       0[15:0]  0[31:16]  1[15:0]   1[31:16]
 //       RGB10_A2UI          1x32       0[9:0]   0[19:10]  0[29:20]  0[31:30]
 //       RGBA8UI             1x32       0[7:0]   0[15:8]   0[23:16]  0[31:24]
 //       RG32UI              2x32       0[31:0]  1[31:0]
 //       RG16UI              1x32       0[15:0]  0[31:16] 
 //       RG8UI               1x16       0[7:0]   0[15:8] 
 //       R32UI               1x32       0[31:0]  
 //       R16UI               1x16       0[15:0] 
 //       R8UI                1x8        0[7:0] 
	/*
	if (format == GL_RGBA32F)
		return(std::string("size4x32"));
	if (format == GL_RGBA16F)
		return(std::string("size2x32"));
		if (format == GL_RG32F)
		return(std::string("size2x32"));
	if (format == GL_RG16F)
		return(std::string("size1x32"));
	if (format == GL_R11F_G11F_B10F)
		return(std::string("size1x32"));
		if (format == GL_R32F)
		return(std::string("size1x32"));
	if (format == GL_R16F)
		return(std::string("size1x16"));
	if (format == GL_RGBA32UI)
		return(std::string("size4x32"));
		if (format == GL_RGBA16UI)
		return(std::string("size2x32"));
	if (format == GL_RGB10_A2UI)
		return(std::string("size1x32"));
		if (format == GL_RGBA8UI)
		return(std::string("size1x32"));
	if (format == GL_RG32UI)
		return(std::string("size2x32"));
		if (format == GL_RG16UI)
		return(std::string("size1x32"));
	if (format == GL_RG8UI)
		return(std::string("size1x16"));
		if (format == GL_R32UI)
		return(std::string("size1x32"));
	if (format == GL_R16UI)
		return(std::string("size1x16"));
	if (format == GL_R8UI)
		return(std::string("size1x8"));

 //       RGBA32I             4x32       0[31:0]  1[31:0]   2[31:0]   3[31:0]
 //       RGBA16I             2x32       0[15:0]  0[31:16]  1[15:0]   1[31:16]
 //       RGBA8I              1x32       0[7:0]   0[15:8]   0[23:16]  0[31:24]
 //       RG32I               2x32       0[31:0]  1[31:0]
 //       RG16I               1x32       0[15:0]  0[31:16] 
 //       RG8I                1x16       0[7:0]   0[15:8] 
 //       R32I                1x32       0[31:0]  
 //       R16I                1x16       0[15:0] 
 //       R8I                 1x8        0[7:0] 

	if (format == GL_RGBA32I)
		return(std::string("size4x32"));
	if (format == GL_RGBA16I)
		return(std::string("size2x32"));
	if (format == GL_RGBA8I)
		return(std::string("size1x32"));
		if (format == GL_RG32I)
		return(std::string("size2x32"));
	if (format == GL_RG16I)
		return(std::string("size1x32"));
		if (format == GL_RG8I)
		return(std::string("size1x16"));
	if (format == GL_R32I)
		return(std::string("size1x32"));
		if (format == GL_R16I)
		return(std::string("size1x16"));	
	if (format == GL_R8I)
		return(std::string("size1x8"));

 //       RGBA16              2x32       0[15:0]  0[31:16]  1[15:0]   1[31:16]
 //       RGB10_A2            1x32       0[9:0]   0[19:10]  0[29:20]  0[31:30]
 //       RGBA8               1x32       0[7:0]   0[15:8]   0[23:16]  0[31:24]
 //       RG16                1x32       0[15:0]  0[31:16] 
 //       RG8                 1x16       0[7:0]   0[15:8] 
 //       R16                 1x16       0[15:0] 
 //       R8                  1x8        0[7:0] 
	if (format == GL_RGBA16)
		return(std::string("size2x32"));
	if (format == GL_RGB10_A2)
		return(std::string("size1x32"));
	if (format == GL_RGBA8)
		return(std::string("size1x32"));
		if (format == GL_RG16)
		return(std::string("size1x32"));
	if (format == GL_RG8)
		return(std::string("size1x16"));
		if (format == GL_R16)
		return(std::string("size1x16"));
	if (format == GL_R8)
		return(std::string("size1x8"));
 //       RGBA16_SNORM        2x32       0[15:0]  0[31:16]  1[15:0]   1[31:16]
 //       RGBA8_SNORM         1x32       0[7:0]   0[15:8]   0[23:16]  0[31:24]
 //       RG16_SNORM          1x32       0[15:0]  0[31:16] 
 //       RG8_SNORM           1x16       0[7:0]   0[15:8] 
 //       R16_SNORM           1x16       0[15:0] 
 //       R8_SNORM            1x8        0[7:0] 

	if (format == GL_RGBA16_SNORM)
		return(std::string("size2x32"));
	if (format == GL_RGBA8_SNORM)
		return(std::string("size1x32"));
	if (format == GL_RG16_SNORM)
		return(std::string("size1x32"));
		if (format == GL_RG8_SNORM)
		return(std::string("size1x16"));
	if (format == GL_R16_SNORM)
		return(std::string("size1x16"));
	if (format == GL_R8_SNORM)
		return(std::string("size1x8"));
		*/



//



	return(std::string("Error : unknown format"));
}

int GPUTexImage::formatToType(int format)
{
	std::string sformat = formatToString();
	if (sformat.rfind("ui") != string::npos)
		return(2);
	else if (sformat.rfind("i") != string::npos)
		return(1);
	else 
		return(0);
		
}