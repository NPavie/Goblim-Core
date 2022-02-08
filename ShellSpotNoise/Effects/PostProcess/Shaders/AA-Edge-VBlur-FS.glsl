/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 440 core

uniform sampler2D EdgeSampler;

in vec2 texCoord;

layout(std140) uniform CPU
{   
    float VStep;
};

layout (location = 0) out float Edge;

void main()
{
	vec2 offsetCoords;
    Edge = 0.0;

	for(float y = -3; y <= 3; ++y)
	{
		offsetCoords = texCoord.xy + vec2(0.0, (y * VStep));
		if(offsetCoords.y >= 0.0 && offsetCoords.y <= 1.0)
			Edge += texture2D(EdgeSampler, offsetCoords).r; 
	}

	Edge /= 7.0;
}