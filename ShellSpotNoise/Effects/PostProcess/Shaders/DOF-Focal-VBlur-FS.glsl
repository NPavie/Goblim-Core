/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 440 core

uniform sampler2D FocalSampler;

in vec2 texCoord;

layout(std140) uniform CPU
{   
	int dofFocalBlurSize;
    float VStep;
};

layout (location = 0) out float Focal;

void main()
{
	vec2 offsetCoords;
	float halfBlurSize = floor(dofFocalBlurSize / 2.0);
	Focal = 0.0;

	for(float y = -halfBlurSize; y <= halfBlurSize; ++y)
	{
		offsetCoords = texCoord.xy + vec2(0.0, (y * VStep));
		if(offsetCoords.y >= 0.0 && offsetCoords.y <= 1.0)
			Focal += texture2D(FocalSampler, offsetCoords).r; 
	}

	Focal /= dofFocalBlurSize;
}