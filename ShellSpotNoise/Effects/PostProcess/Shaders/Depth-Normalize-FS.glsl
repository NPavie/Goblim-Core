/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 440 core

uniform sampler2D DepthSampler;

in vec2 texCoord;

layout(std140) uniform CPU
{	
	float zNear;
	float zFar;
};

layout (location = 0) out float Depth;

float linearize(float depth)
{
	return (2 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

void main()
{	
	Depth = linearize(texture(DepthSampler, texCoord).r);
}