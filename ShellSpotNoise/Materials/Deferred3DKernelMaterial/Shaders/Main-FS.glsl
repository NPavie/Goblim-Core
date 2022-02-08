#version 430
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Kernels"
#line 5



layout(std140) uniform CPU
{
	mat4 objectToScreen; 	// MVP
	mat4 objectToCamera; 	// MV
	mat4 objectToWorld;		// M
	vec3 metalMask_Reflectance_Roughness;
};
uniform sampler2DArray smp_models;

uniform sampler2D smp_depthBuffer;




in vec4 v_position;
in vec3 v_normal;
in vec3 v_textureCoords;
in vec3 v_CameraSpaceCoord;
in vec3 surface_position;
in vec3 surface_normale;
in mat4 kernelToObjectSpace;
flat in int kernelTextureID;
in kernel computeKernel;

in float gridHeight;

flat in int v_invalidKernel;

in vec4 kernelBaseColor;

in vec4 g_ScreenPosition;
in vec4 g_ScreenLastPosition;

//layout (location = 0) out vec4 Color;
layout (location = 0) out vec3 AOColor;
layout (location = 1) out vec3 BaseColor;
layout (location = 2) out vec3 MetalMask_Reflectance_Roughness;
layout (location = 3) out vec4 WorldPosition;
layout (location = 4) out vec3 WorldNormal;
layout (location = 5) out vec2 ScreenVelocity;
layout (location = 6) out vec3 depthInViewSpace;



void main()
{
	// Depth buffer uniquement pour rendu en postprocess
	//float zBufferDepth = textureLod(smp_depthBuffer,gl_FragCoord.xy / vec2(1920.0,1080.0),0).x;
	//if(-v_CameraSpaceCoord.z > zBufferDepth || v_invalidKernel > 0.1 ) discard;
	if(v_invalidKernel > 0.1 ) discard;
	// Pour le rayCasting : créé une matrice quadratic a partir

	mat4 KernelToCameraSpace = objectToCamera * kernelToObjectSpace;
	mat4 CameraToWordSplace = objectToWorld * inverse(objectToCamera);
	
	vec3 intersectedPosition = vec3(0.5);
	vec3 intersectedNormal = normalize(vec3(1.0));
	
	bool isIntersected = intersection_quadraticKernel(
							vec3(0.0), 
						    computeKernel.data[1].xyz,
						    KernelToCameraSpace * v_position,
						    KernelToCameraSpace,
						    intersectedPosition,
						    intersectedNormal
							);
	


	if(isIntersected == false) discard;
	



	AOColor = vec3(1.0);
	BaseColor = colorFromIntRGB(255,255,255).rgb;
	MetalMask_Reflectance_Roughness = metalMask_Reflectance_Roughness;					// TO ADD
	WorldPosition = (CameraToWordSplace * vec4(intersectedPosition,1.0));
	WorldNormal = normalize( (CameraToWordSplace * vec4(intersectedNormal,1.0)).xyz );

	// Compute screen velocity for Per Object Motion Blur
	vec2 a = (g_ScreenPosition.xy / g_ScreenPosition.w) * 0.5 + 0.5;
    vec2 b = (g_ScreenLastPosition.xy / g_ScreenLastPosition.w) * 0.5 + 0.5;
    ScreenVelocity = (a - b);
	
	vec3 objectPosition = (inverse(objectToWorld) * WorldPosition).xyz;
	vec3 VV = (objectPosition.xyz - surface_position.xyz);
	float height = dot(normalize(surface_normale.xyz),VV);
    
    AOColor = vec3(0.5 + 0.5 * (height / gridHeight));

	depthInViewSpace = vec3(-v_CameraSpaceCoord.z,0,1);

}

