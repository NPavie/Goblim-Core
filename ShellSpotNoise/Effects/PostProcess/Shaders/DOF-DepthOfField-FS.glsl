/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 440 core

uniform sampler2D ColorSampler;
uniform sampler2D BlurSampler;
uniform sampler2D FocalSampler;

in vec2 texCoord;

layout (location = 0) out vec4 Color;

void main()
{	    
	Color = mix(texture2D(ColorSampler, texCoord), texture2D(BlurSampler, texCoord), texture2D(FocalSampler, texCoord).r);
}