/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 440 core

uniform sampler2D EdgeSampler;
uniform sampler2D ShadingSampler;

in vec2 texCoord;

layout(std140) uniform CPU
{	
    float VStep;
};

layout (location = 0) out vec3 Color;

void main()
{
	vec2 offsetCoords;
	Color = vec3(0.0);
	
	float edge; 
	float edgeSum = 0.0;
	if(texture2D(EdgeSampler, texCoord).r > 0.0)
	{
		for(float y = -1; y <= 1; ++y)
		{
			offsetCoords = texCoord.xy + vec2(0.0, (y * VStep));
			if(offsetCoords.y >= 0.0 && offsetCoords.y <= 1.0)
			{
				edge = texture2D(EdgeSampler, offsetCoords).r;
				edgeSum += edge;
				Color += texture2D(ShadingSampler, offsetCoords).rgb * edge; 
			}
		}
		Color /= edgeSum;
	}
}