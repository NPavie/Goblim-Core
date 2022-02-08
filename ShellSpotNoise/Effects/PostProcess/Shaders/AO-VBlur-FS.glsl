/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 440 core

uniform sampler2D AOSampler;

in vec2 texCoord;

layout(std140) uniform CPU
{	
	int aoBlurSize;
    float VStep;
};

layout (location = 0) out float AO;

void main()
{
	AO = 0.0;
	vec2 offsetCoords;
	float halfBlurSize = floor(aoBlurSize / 2.0);

	for(float y = -halfBlurSize; y <= halfBlurSize; ++y)
	{
		offsetCoords = texCoord.xy + vec2(0.0, (y * VStep));
		if(offsetCoords.y >= 0.0 && offsetCoords.y <= 1.0)
			AO += texture(AOSampler, offsetCoords).r;
	}

	AO = AO / aoBlurSize;
}