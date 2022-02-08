/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 440 core

uniform sampler2D FocalSampler;
uniform sampler2D ShadingSampler;

in vec2 texCoord;

layout(std140) uniform CPU
{	
	int dofShadingBlurSize;
	//float dofFocalMultiplier;
    float VStep;
};

layout (location = 0) out vec3 Color;

void main()
{
	float dofFocalMultiplier = 8.0;
	float depth = texture2D(FocalSampler, texCoord).r;

	float BlurSize = (dofShadingBlurSize / 2.0);// * (depth * dofFocalMultiplier);

	float halfBlurSize = floor(BlurSize);

	Color = vec3(0.0);
	vec2 offsetCoords;

	if(depth > 0.0)
	{
		for(float y = -halfBlurSize; y <= halfBlurSize; ++y)
		{
			offsetCoords = texCoord.xy + vec2(0.0, (y * VStep));
			if(offsetCoords.y >= 0.0 && offsetCoords.y <= 1.0)
				Color += texture2D(ShadingSampler, offsetCoords).rgb; 
		}
		Color /= max(BlurSize*2.0, 1.0);
	}
}