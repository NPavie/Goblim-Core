/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 440 core

uniform sampler2D ColorSampler;
uniform sampler2D DepthSampler;

in vec2 texCoord;

layout(std140) uniform CPU
{	
	float zNear;
	float zFar;
};

layout (location = 0) out vec3 Color;

// Fog Factor
float ComputeFogFactor(in float Depth)
{
	return (0.66 + (0.33 * Depth) + 0.17 * pow(Depth, 2.0));
}

float linearize(float depth)
{
	return (2 * zNear) / (zFar + zNear - depth * (zFar - zNear));
}

void main()
{
	float grayScale = 192.0 / 255.0;
	
	float Z = linearize(texture(DepthSampler, texCoord).x);
	
	Color = mix(texture(ColorSampler, texCoord).rgb, vec3(grayScale), clamp(ComputeFogFactor(Z), 0.0, 1.0)).rgb;
}