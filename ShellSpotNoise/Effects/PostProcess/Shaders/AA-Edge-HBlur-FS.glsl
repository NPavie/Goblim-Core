/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 440 core

uniform sampler2D EdgeSampler;

in vec2 texCoord;

layout(std140) uniform CPU
{	
    float HStep;
};

layout (location = 0) out float Edge;

void main()
{
	vec2 offsetCoords;
	Edge = 0.0;

	for(float x = -3; x <= 3; ++x)
	{
		offsetCoords = texCoord.xy + vec2((x * HStep), 0.0);
		if(offsetCoords.x >= 0.0 && offsetCoords.x <=1.0)
			Edge += texture2D(EdgeSampler, offsetCoords).r; 
	}

	Edge /= 7.0;
}