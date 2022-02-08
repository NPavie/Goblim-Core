/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 440 core

uniform sampler2D ColorSampler;
uniform sampler2D BlurSampler;

in vec2 texCoord;

layout (location = 0) out vec3 Color;

void main()
{	    
    Color = texture2D(ColorSampler, texCoord).rgb + texture2D(BlurSampler, texCoord).rgb;
}