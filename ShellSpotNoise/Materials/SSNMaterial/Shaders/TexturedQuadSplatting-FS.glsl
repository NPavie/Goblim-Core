#version 430
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Common"
#include "/Materials/Common/Kernels"						
#line 6

// constantes
#ifndef PI 
	#define PI 3.14159265359
	#define PI_2 1.57079632679
	#define PI_4 0.78539816339
#endif


layout(std140) uniform CPU
{
	mat4 objectToScreen; 	// MVP
	mat4 objectToCamera; 	// MV
	mat4 objectToWorld;		// M
};
uniform sampler2DArray smp_models;

uniform sampler2D smp_depthBuffer;


in vec4 v_position;
in vec3 v_normal;
in vec3 v_textureCoords;
in vec3 surface_position;
in vec3 surface_normale;
in mat4 kernelToObjectSpace;
flat in int kernelTextureID;
in kernel computeKernel;

in float gridHeight;

flat in int v_invalidKernel;

in vec4 kernelBaseColor;

layout (location = 0) out vec4 Color;


// test 
void main()
{
	if(v_invalidKernel > 0.1) discard;
	vec4 z_b = textureLod(smp_depthBuffer,gl_FragCoord.xy / vec2(1280.0,720.0),0);

	bool isValid = false;
	float depthValue = 0.0;
	float height = 0.0;
	vec4 test = getKernelColor(
			computeKernel,
			vec3(v_textureCoords.xy,0.0),
			v_textureCoords + dFdx(v_textureCoords),
			v_textureCoords + dFdy(v_textureCoords),
			smp_models,
			kernelTextureID,
			kernelToObjectSpace,
			objectToWorld,
			objectToCamera,
			z_b.w > 0.0f,
			z_b.x,
			surface_position,
			surface_normale,
			gridHeight,
			depthValue,
			height,
			isValid );

	if(!isValid) discard;


	float oitWeight = weightFunction(depthValue) * shellWeightFunction(height / gridHeight);
	float newAlpha = test.a * oitWeight;
	Color = vec4(test.rgb * newAlpha , newAlpha);// * kernelColor.w;


}