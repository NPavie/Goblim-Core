#version 430

#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/SpotTree"
#line 6 

layout(std140) uniform CPU
{
	int impulseDensityPerCell;
	vec2 resolution;
};



in vec3 worldNormal;
in vec3 worldPosition;
in vec3 textureCoordinates;

out vec4 Color;

void main()
{		
	// test d'évaluation d'un spot
	//float testValue = surface_noise_testSimpleSpot(textureCoordinates.xy);
	//float testValue = evaluateNoiseNode(textureCoordinates.xy, 0);
	float testValue = evaluateNoiseTest(textureCoordinates.xy);

	Color = vec4(testValue,testValue,testValue,1.0);

}

