/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 440 core

uniform sampler2D BrightSampler;

in vec2 texCoord;

layout(std140) uniform CPU
{	
	int blBlurSize;
    float VStep;
};

layout (location = 0) out vec3 Color;

void main()
{
	Color = vec3(0.0);
	vec2 offsetCoords;
	float halfBlurSize = floor(blBlurSize / 2.0);

	for(float y = -halfBlurSize; y <= halfBlurSize; ++y)
	{
		offsetCoords = texCoord.xy + vec2(0.0, (y * VStep));
		if(offsetCoords.y >= 0.0 && offsetCoords.y <= 1.0)
			Color += texture2D(BrightSampler, offsetCoords).rgb;
	}

	Color = Color / blBlurSize;
}