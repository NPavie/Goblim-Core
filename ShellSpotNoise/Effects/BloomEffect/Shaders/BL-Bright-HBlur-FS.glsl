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
    float HStep;
};

layout (location = 0) out vec3 Color;

void main()
{
	Color = vec3(0.0);
	vec2 offsetCoords;
	float halfBlurSize = floor(blBlurSize / 2.0);

	for(float x = -halfBlurSize; x <= halfBlurSize; ++x)
	{
		offsetCoords = texCoord.xy + vec2((x * HStep), 0.0);
		if(offsetCoords.x >= 0.0 && offsetCoords.x <=1.0)
			Color += texture(BrightSampler, offsetCoords).rgb;
	}

	Color = Color / blBlurSize;
}