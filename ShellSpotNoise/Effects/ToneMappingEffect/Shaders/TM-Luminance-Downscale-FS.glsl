/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 440 core

uniform sampler2D LuminanceSampler;

layout(std140) uniform CPU
{	
	float HStep;
	float VStep;
};

in vec2 texCoord;

layout (location = 0) out float MaxRGBValue;

void main()
{	
    float a = texture2D(LuminanceSampler, texCoord						).r;
    float b = texture2D(LuminanceSampler, texCoord + vec2(HStep	, 0.0)	).r;
    float c = texture2D(LuminanceSampler, texCoord + vec2(HStep	, VStep)).r;
    float d = texture2D(LuminanceSampler, texCoord + vec2(0.0	, VStep)).r;
    
    MaxRGBValue = max(max(a, b), max(c, d));
}