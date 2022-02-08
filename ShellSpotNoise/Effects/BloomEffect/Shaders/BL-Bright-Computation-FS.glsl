/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 440 core

uniform sampler2D ColorSampler;

layout(std140) uniform CPU
{	
	float Threshold;
};

in vec2 texCoord;

layout (location = 0) out vec3 Color;

void main()
{
    vec3 inColor = texture2D(ColorSampler, texCoord).rgb;
    
    float MaxRGBValue = max(inColor.r, max(inColor.g, inColor.b));
    
    if(MaxRGBValue > Threshold)
    {
        Color = vec3(inColor);
    }
	else
	{
		Color = vec3(0.0);
	}
}