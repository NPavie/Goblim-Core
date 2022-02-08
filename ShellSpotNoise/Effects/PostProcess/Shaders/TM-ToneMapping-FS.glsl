/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 440 core

uniform sampler2D HDRSampler;

in vec2 texCoord;

layout(std140) uniform CPU
{	
	float tmMaxRGBValue;
	float tmExposure;
};

layout (location = 0) out vec3 Color;

void main()
{	    
    
	vec3 col = texture(HDRSampler, texCoord).rgb;

	#if 0
	if(texCoord.x<0.33)
    	Color = col;
    else if(texCoord.x>0.33&&texCoord.x<0.66)
    	Color =  col / (col+vec3(1.0));
    else {
    	float Y = dot(vec4(0.30, 0.59, 0.11, 0.0), vec4(col, 1.0));
		float YD = tmExposure * (tmExposure/tmMaxRGBValue + 1.0) / (tmExposure + 1.0);
		col *= YD;
    	Color = col;
	}
	#else 
	   	float Y = dot(vec4(0.30, 0.59, 0.11, 0.0), vec4(col, 1.0));
		float YD = tmExposure * (tmExposure/tmMaxRGBValue + 1.0) / (tmExposure + 1.0);
		col *= YD;
    	Color = col;
	#endif

}