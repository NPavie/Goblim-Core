/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 440 core

uniform sampler2D ColorSampler;
uniform sampler2D EdgeSampler;
uniform sampler2D BlurSampler;

in vec2 texCoord;

layout (location = 0) out vec3 Color;

void main()
{
	float Edge = texture2D(EdgeSampler, texCoord).r;

	if(Edge > 0.0)
	{
		Color = texture2D(BlurSampler, texCoord).rgb;
	}
	else
	{
		Color = texture2D(ColorSampler, texCoord).rgb;
	}
}