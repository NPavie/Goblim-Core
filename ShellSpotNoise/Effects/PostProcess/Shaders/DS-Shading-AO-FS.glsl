#version 440 core
#extension GL_ARB_shader_storage_buffer_object : enable
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Lighting/Lighting"

uniform sampler2DArray GBuffers;
uniform sampler2D DOSampler;

in vec2 texCoord;
layout(std140) uniform CPU
{	
	vec3 CamPos;
};

layout (location = 0) out vec3 Color;

void main()
{
	//----- Ambient Occlusion -----//
	vec3 DO = textureLod(DOSampler, texCoord, 0).rgb;
	//----- Albedo : Diffuse -----//
	vec3 BaseColor = texture(GBuffers, vec3(texCoord.xy, 1.0)).rgb;
	//----- Specular -----//
	vec3 MetalMask_Reflectance_Roughness = texture(GBuffers, vec3(texCoord.xy, 2.0)).rgb;
	//----- Position -----//
	vec3 Position = texture(GBuffers, vec3(texCoord.xy, 3.0)).xyz;
	//----- Normal -----//
	vec3 N = texture(GBuffers, vec3(texCoord.xy, 4.0)).xyz;
	

	float metalMask = MetalMask_Reflectance_Roughness.r;
	float reflectance = MetalMask_Reflectance_Roughness.g;
	float roughness = MetalMask_Reflectance_Roughness.b;

	vec3 F0 = 0.16f*(reflectance*reflectance)*(1.0f - metalMask) + BaseColor*metalMask; // Idem FrostBite


	if(N != vec3(0.0) && N != vec3(1.0)) 
		Color = addPBR( Position, N, normalize (CamPos.xyz-Position), BaseColor, roughness, F0, DO).rgb; // Calcul de la couleur du pixel
	else Color = vec3(0);

	
}