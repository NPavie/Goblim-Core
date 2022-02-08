
#include "ImageUtils.h"

extern "C"
{
 #include "image_DXT.h"
}
#define SOIL_RGB_S3TC_DXT1		0x83F0
#define SOIL_RGBA_S3TC_DXT1		0x83F1
#define SOIL_RGBA_S3TC_DXT3		0x83F2
#define SOIL_RGBA_S3TC_DXT5		0x83F3



#ifdef WIN32
// Because microsoft compiler doesn't provide log2 
unsigned int log2( unsigned int x )
{
	unsigned int ans = 0 ;
	while( x>>=1 ) ans++;
	return ans ;
}
#endif




int
	mipmap_image
	(
		const unsigned char* const orig,
		int width, int height, int channels,
		unsigned char* resampled,
		int block_size_x, int block_size_y
	)
{
	int mip_width, mip_height;
	int i, j, c;

	/*	error check	*/
	if( (width < 1) || (height < 1) ||
		(channels < 1) || (orig == NULL) ||
		(resampled == NULL) ||
		(block_size_x < 1) || (block_size_y < 1) )
	{
		/*	nothing to do	*/
		return 0;
	}
	mip_width = width / block_size_x;
	mip_height = height / block_size_y;
	if( mip_width < 1 )
	{
		mip_width = 1;
	}
	if( mip_height < 1 )
	{
		mip_height = 1;
	}
	for( j = 0; j < mip_height; ++j )
	{
		for( i = 0; i < mip_width; ++i )
		{
			for( c = 0; c < channels; ++c )
			{
				const int index = (j*block_size_y)*width*channels + (i*block_size_x)*channels + c;
				int sum_value;
				int u,v;
				int u_block = block_size_x;
				int v_block = block_size_y;
				int block_area;
				/*	do a bit of checking so we don't over-run the boundaries
					(necessary for non-square textures!)	*/
				if( block_size_x * (i+1) > width )
				{
					u_block = width - i*block_size_y;
				}
				if( block_size_y * (j+1) > height )
				{
					v_block = height - j*block_size_y;
				}
				block_area = u_block*v_block;
				/*	for this pixel, see what the average
					of all the values in the block are.
					note: start the sum at the rounding value, not at 0	*/
				sum_value = block_area >> 1;
				for( v = 0; v < v_block; ++v )
				for( u = 0; u < u_block; ++u )
				{
					sum_value += orig[index + v*width*channels + u*channels];
				}
				resampled[j*mip_width*channels + i*channels + c] = sum_value / block_area;
			}
		}
	}
	return 1;
}






unsigned int loadLayer	(const unsigned char *const data,int width, int height, int channels,int layer,int target_width,int target_height,unsigned int flags)
{
	/*	variables	*/
	unsigned char* img;	
	unsigned int internal_texture_format = 0, original_texture_format = 0;	

	/*	create a copy the image data	*/
	img = (unsigned char*)malloc( width*height*channels );
	memcpy( img, data, width*height*channels );
	
	if( flags & SOIL_FLAG_INVERT_Y )
	{
		int i, j;
		for( j = 0; j*2 < height; ++j )
		{
			int index1 = j * width * channels;
			int index2 = (height - 1 - j) * width * channels;
			for( i = width * channels; i > 0; --i )
			{
				unsigned char temp = img[index1];
				img[index1] = img[index2];
				img[index2] = temp;
				++index1;
				++index2;
			}
		}
	}

	img = resize(img,width,channels,target_width);
	width = target_width;
	height = target_height;


		/*	and what type am I using as the internal texture format?	*/
		switch( channels )
		{
		case 1:
			original_texture_format = GL_LUMINANCE;
			break;
		case 2:
			original_texture_format = GL_LUMINANCE_ALPHA;
			break;
		case 3:
			original_texture_format = GL_RGB;
			break;
		case 4:
			original_texture_format = GL_RGBA;
			break;
		}
		internal_texture_format = original_texture_format;
		/*	does the user want me to, and can I, save as DXT?	*/

		if (flags & SOIL_FLAG_COMPRESS_TO_DXT)
		{
		
		/*	I can use DXT, whether I compress it or OpenGL does	*/
				if( (channels & 1) == 1 )
				{
					/*	1 or 3 channels = DXT1	*/
					internal_texture_format = SOIL_RGB_S3TC_DXT1;
				} else
				{
					/*	2 or 4 channels = DXT5	*/
					internal_texture_format = SOIL_RGBA_S3TC_DXT5;
				}
		/*	user wants me to do the DXT conversion!	*/
			int DDS_size;
			unsigned char *DDS_data = NULL;
			if( (channels & 1) == 1 )
			{
				/*	RGB, use DXT1	*/
				DDS_data = convert_image_to_DXT1( img, width, height, channels, &DDS_size );
			} else
			{
				/*	RGBA, use DXT5	*/
				DDS_data = convert_image_to_DXT5( img, width, height, channels, &DDS_size );
			}
			if( DDS_data )
			{

				glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,0,0,layer, width, height,1, internal_texture_format,
					DDS_size, DDS_data);

				SOIL_free_image_data( DDS_data );
				/*	printf( "Internal DXT compressor\n" );	*/
			} 
		}
			else
			{
				/*	my compression failed, try the OpenGL driver's version	*/
			
				glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,0,0,layer, width, height,1, internal_texture_format,GL_UNSIGNED_BYTE,img);
			//	check_for_GL_errors( "glTexImage2D" );
				/*	printf( "OpenGL DXT compressor\n" );	*/
			}
		
		if (flags & SOIL_FLAG_MIPMAPS)
		{
		
			int MIPlevel = 1;
			int MIPwidth = (width+1) / 2;
			int MIPheight = (height+1) / 2;
			unsigned char *resampled = (unsigned char*)malloc( channels*MIPwidth*MIPheight );
			while( ((1<<MIPlevel) <= width) || ((1<<MIPlevel) <= height) )
			{
				/*	do this MIPmap level	*/
				mipmap_image(
						img, width, height, channels,
						resampled,
						(1 << MIPlevel), (1 << MIPlevel) );
				/*  upload the MIPmaps	*/

				if (flags & SOIL_FLAG_COMPRESS_TO_DXT)
				{
				
					/*	user wants me to do the DXT conversion!	*/
					int DDS_size;
					unsigned char *DDS_data = NULL;
					if( (channels & 1) == 1 )
					{
						/*	RGB, use DXT1	*/
						DDS_data = convert_image_to_DXT1(
								resampled, MIPwidth, MIPheight, channels, &DDS_size );
					} else
					{
						/*	RGBA, use DXT5	*/
						DDS_data = convert_image_to_DXT5(
								resampled, MIPwidth, MIPheight, channels, &DDS_size );
					}
					if( DDS_data )
					{
											
						glCompressedTexSubImage3D(GL_TEXTURE_2D_ARRAY_EXT, MIPlevel,0,0,layer, MIPwidth, MIPheight,1, internal_texture_format,
							DDS_size, DDS_data);


						//check_for_GL_errors( "glCompressedTexImage2D" );
						SOIL_free_image_data( DDS_data );
					} 
				}
				else
				{
					
					glTexSubImage3D(GL_TEXTURE_2D_ARRAY_EXT, MIPlevel,0,0,layer, MIPwidth, MIPheight,1, internal_texture_format,GL_UNSIGNED_BYTE,img);
				}
				
				/*	prep for the next level	*/
				++MIPlevel;
				MIPwidth = (MIPwidth + 1) / 2;
				MIPheight = (MIPheight + 1) / 2;
			}
			SOIL_free_image_data( resampled );
		}
	SOIL_free_image_data( img );
		
	return true;
}



unsigned char* resize(unsigned char* orig,int size,int channels,int resampled_size)
{
	int width = size;
	int height = size;
	unsigned char *resampled = (unsigned char*)malloc( channels*resampled_size*resampled_size );
	if (size > resampled_size)
	{
		
		int reduce_block_x = 1, reduce_block_y = 1;
		int new_width, new_height;
		if( width > resampled_size )
		{
			reduce_block_x = width / resampled_size;
		}
		if( height > resampled_size )
		{
			reduce_block_y = height / resampled_size;
		}
		new_width = width / reduce_block_x;
		new_height = height / reduce_block_y;
		/*	perform the actual reduction	*/
		mipmap_image(	orig, width, height, channels,
						resampled, reduce_block_x, reduce_block_y );
	}
	else if (size < resampled_size)
		up_scale_image(orig,width, height, channels,resampled,resampled_size, resampled_size);
	else
		memcpy( resampled, orig, width*height*channels );
	SOIL_free_image_data( orig );
	return resampled;

}
int
	up_scale_image
	(
		const unsigned char* const orig,
		int width, int height, int channels,
		unsigned char* resampled,
		int resampled_width, int resampled_height
	)
{
	float dx, dy;
	int x, y, c;

    /* error(s) check	*/
    if ( 	(width < 1) || (height < 1) ||
            (resampled_width < 2) || (resampled_height < 2) ||
            (channels < 1) ||
            (NULL == orig) || (NULL == resampled) )
    {
        /*	signify badness	*/
        return 0;
    }
    /*
		for each given pixel in the new map, find the exact location
		from the original map which would contribute to this guy
	*/
    dx = (width - 1.0f) / (resampled_width - 1.0f);
    dy = (height - 1.0f) / (resampled_height - 1.0f);
    for ( y = 0; y < resampled_height; ++y )
    {
    	/* find the base y index and fractional offset from that	*/
    	float sampley = y * dy;
    	int inty = (int)sampley;
    	/*	if( inty < 0 ) { inty = 0; } else	*/
		if( inty > height - 2 ) { inty = height - 2; }
		sampley -= inty;
        for ( x = 0; x < resampled_width; ++x )
        {
			float samplex = x * dx;
			int intx = (int)samplex;
			int base_index;
			/* find the base x index and fractional offset from that	*/
			/*	if( intx < 0 ) { intx = 0; } else	*/
			if( intx > width - 2 ) { intx = width - 2; }
			samplex -= intx;
			/*	base index into the original image	*/
			base_index = (inty * width + intx) * channels;
            for ( c = 0; c < channels; ++c )
            {
            	/*	do the sampling	*/
				float value = 0.5f;
				value += orig[base_index]
							*(1.0f-samplex)*(1.0f-sampley);
				value += orig[base_index+channels]
							*(samplex)*(1.0f-sampley);
				value += orig[base_index+width*channels]
							*(1.0f-samplex)*(sampley);
				value += orig[base_index+width*channels+channels]
							*(samplex)*(sampley);
				/*	move to the next channel	*/
				++base_index;
            	/*	save the new value	*/
            	resampled[y*resampled_width*channels+x*channels+c] =
						(unsigned char)(value);
            }
        }
    }
    /*	done	*/
    return 1;
}


unsigned int loadSimpleLayer(const unsigned char *const data,int width, int height, int channels,unsigned int flags,
		int layer,
		int target_size
	)
{
	/*	variables	*/
	unsigned char* img;
	
	unsigned int internal_texture_format = 0, original_texture_format = 0;
	

	/*	create a copy the image data	*/
	img = (unsigned char*)malloc( width*height*channels );
	memcpy( img, data, width*height*channels );
	
	if( flags & SOIL_FLAG_INVERT_Y )
	{
		int i, j;
		for( j = 0; j*2 < height; ++j )
		{
			int index1 = j * width * channels;
			int index2 = (height - 1 - j) * width * channels;
			for( i = width * channels; i > 0; --i )
			{
				unsigned char temp = img[index1];
				img[index1] = img[index2];
				img[index2] = temp;
				++index1;
				++index2;
			}
		}
	}

		/*	and what type am I using as the internal texture format?	*/
		switch( channels )
		{
		case 1:
			original_texture_format = GL_LUMINANCE;
			break;
		case 2:
			original_texture_format = GL_LUMINANCE_ALPHA;
			break;
		case 3:
			original_texture_format = GL_RGB;
			break;
		case 4:
			original_texture_format = GL_RGBA;
			break;
		}	
		

		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0,0,0,layer, width, height,1, original_texture_format,GL_UNSIGNED_BYTE, img);
		
		SOIL_free_image_data( img );
	return 0;
}


GLint load2DTexture(std::string filename,unsigned int flags)
{
/*	variables	*/
	unsigned int internal_texture_format = 0, original_texture_format = 0;	

	

	int width,height,channels;


	unsigned char * img = SOIL_load_image(filename.c_str(),&width, &height, &channels,flags);
	int level = log2(width)+1;


	
	if( flags & SOIL_FLAG_INVERT_Y )
	{
		int i, j;
		for( j = 0; j*2 < height; ++j )
		{
			int index1 = j * width * channels;
			int index2 = (height - 1 - j) * width * channels;
			for( i = width * channels; i > 0; --i )
			{
				unsigned char temp = img[index1];
				img[index1] = img[index2];
				img[index2] = temp;
				++index1;
				++index2;
			}
		}
	}



		/*	and what type am I using as the internal texture format?	*/
		switch( channels )
		{
		case 1:
			original_texture_format = GL_LUMINANCE;
			break;
		case 2:
			original_texture_format = GL_LUMINANCE_ALPHA;
			break;
		case 3:
			original_texture_format = GL_RGB;
			break;
		case 4:
			original_texture_format = GL_RGBA;
			break;
		}
		internal_texture_format = original_texture_format;
		/*	does the user want me to, and can I, save as DXT?	*/

if (flags & SOIL_FLAG_COMPRESS_TO_DXT)
		{
		
		/*	I can use DXT, whether I compress it or OpenGL does	*/
				if( (channels & 1) == 1 )
				{
					/*	1 or 3 channels = DXT1	*/
					internal_texture_format = SOIL_RGB_S3TC_DXT1;
				} else
				{
					/*	2 or 4 channels = DXT5	*/
					internal_texture_format = SOIL_RGBA_S3TC_DXT5;
				}
			}

	
	GLuint texId;
	glGenTextures(1,&texId);
	glBindTexture(GL_TEXTURE_2D,texId);
	glTexStorage2D(GL_TEXTURE_2D,level,GL_RGB8,width,height);




		if (flags & SOIL_FLAG_COMPRESS_TO_DXT)
		{
		
		/*	I can use DXT, whether I compress it or OpenGL does	*/
				if( (channels & 1) == 1 )
				{
					/*	1 or 3 channels = DXT1	*/
					internal_texture_format = SOIL_RGB_S3TC_DXT1;
				} else
				{
					/*	2 or 4 channels = DXT5	*/
					internal_texture_format = SOIL_RGBA_S3TC_DXT5;
				}
		/*	user wants me to do the DXT conversion!	*/
			int DDS_size;
			unsigned char *DDS_data = NULL;
			if( (channels & 1) == 1 )
			{
				/*	RGB, use DXT1	*/
				DDS_data = convert_image_to_DXT1( img, width, height, channels, &DDS_size );
			} else
			{
				/*	RGBA, use DXT5	*/
				DDS_data = convert_image_to_DXT5( img, width, height, channels, &DDS_size );
			}
			if( DDS_data )
			{

				glCompressedTexSubImage2D(GL_TEXTURE_2D, 0,0,0, width, height, internal_texture_format,
					DDS_size, DDS_data);

				SOIL_free_image_data( DDS_data );
				/*	printf( "Internal DXT compressor\n" );	*/
			} 
		}
			else
			{
				/*	my compression failed, try the OpenGL driver's version	*/

			
				glTexSubImage2D(GL_TEXTURE_2D,0, 0,0, width, height, GL_RGBA,GL_UNSIGNED_BYTE,img);
			
			//	check_for_GL_errors( "glTexImage2D" );
				/*	printf( "OpenGL DXT compressor\n" );	*/
			}
		
		if (flags & SOIL_FLAG_MIPMAPS)
		{
		
			int MIPlevel = 1;
			int MIPwidth = (width) / 2;
			int MIPheight = (height) / 2;
			unsigned char *resampled = (unsigned char*)malloc( channels*MIPwidth*MIPheight );
			while( ((1<<MIPlevel) <= width) || ((1<<MIPlevel) <= height) )
			{
				/*	do this MIPmap level	*/
				mipmap_image(
						img, width, height, channels,
						resampled,
						(1 << MIPlevel), (1 << MIPlevel) );
				/*  upload the MIPmaps	*/

				if (flags & SOIL_FLAG_COMPRESS_TO_DXT)
				{
				
					/*	user wants me to do the DXT conversion!	*/
					int DDS_size;
					unsigned char *DDS_data = NULL;
					if( (channels & 1) == 1 )
					{
						/*	RGB, use DXT1	*/
						DDS_data = convert_image_to_DXT1(
								resampled, MIPwidth, MIPheight, channels, &DDS_size );
					} else
					{
						/*	RGBA, use DXT5	*/
						DDS_data = convert_image_to_DXT5(
								resampled, MIPwidth, MIPheight, channels, &DDS_size );
					}
					if( DDS_data )
					{
											
						glCompressedTexSubImage2D(GL_TEXTURE_2D, MIPlevel,0,0, MIPwidth, MIPheight, internal_texture_format,
							DDS_size, DDS_data);


						//check_for_GL_errors( "glCompressedTexImage2D" );
						SOIL_free_image_data( DDS_data );
					} 
				}
				else
				{
					
					glTexSubImage2D(GL_TEXTURE_2D, MIPlevel,0,0, MIPwidth, MIPheight, GL_RGBA,GL_UNSIGNED_BYTE,img);
					
				}
				
				/*	prep for the next level	*/
				++MIPlevel;
				MIPwidth = (MIPwidth + 1) / 2;
				MIPheight = (MIPheight + 1) / 2;
			}
			SOIL_free_image_data( resampled );
		}
	SOIL_free_image_data( img );
		
	return texId;



}