/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 440 core

uniform sampler2DArray GBuffers;
uniform sampler2D DepthSampler;
uniform sampler2D NoiseSampler;
uniform vec3 kernel[128];

in vec2 texCoord;

layout(std140) uniform CPU
{	
    float HStep;
    float VStep;
	int numSamples;
};

layout (location = 0) out float AO;

float ComputeAOFactor(in vec3 Origin, in vec3 Normal, in vec3 NoiseValue, in float OriginDepth)
{
	vec3 Sample;
	float SampleDepth;

	float depthDiff;

	float Radius = 0.03125;
	float Occlusion = 0.0;

	for(int i = 0; i < numSamples; ++i)
	{
		Sample = Radius * reflect(kernel[i], NoiseValue);
		Sample = Origin + sign(dot(Sample,Normal)) * Sample;

		SampleDepth = texture(DepthSampler, Sample.xy).r;

		depthDiff = OriginDepth - SampleDepth;

		if(depthDiff > 0.0)
			Occlusion += (1.0 - smoothstep(0.0, Radius, depthDiff));
	}

	return (1.0 - (Occlusion / float(numSamples)));
}

void main()
{	
	//----- Fragment Depth -----//
	float OriginDepth = texture(DepthSampler, texCoord).r;
	//----- Fragment Position -----//
	vec3 Origin = vec3(texCoord, OriginDepth);
	//----- Fragment Noise Value -----//
	vec3 NoiseValue = texture(NoiseSampler, texCoord * vec2(HStep, VStep)).rgb;
	//----- Fragment Normal -----//
	vec3 Normal = texture(GBuffers, vec3(texCoord, 4.0)).xyz;

	if(Normal != vec3(0.0) && Normal != vec3(1.0) && OriginDepth < 1.0)
			AO = (ComputeAOFactor(Origin, Normal, NoiseValue, OriginDepth));
	else
	{	
		if(Normal == vec3(1.0))
			AO = (1.0);
		else
			AO = (0.0);
	}
}