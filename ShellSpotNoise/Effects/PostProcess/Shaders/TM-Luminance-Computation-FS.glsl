/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 440 core

uniform sampler2D HDRLuminanceSampler;

in vec2 texCoord;

layout (location = 0) out float MaxRGBValue;

void main()
{	
    vec3 HDRColor = texture2D(HDRLuminanceSampler, texCoord).rgb;   
    MaxRGBValue = max(HDRColor.r, max(HDRColor.g, HDRColor.b));
}