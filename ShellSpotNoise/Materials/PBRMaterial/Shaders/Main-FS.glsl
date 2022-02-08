#version 440

#extension GL_ARB_shading_language_include : enable
#extension GL_NV_shadow_samplers_cube : enable
#include "/Materials/Common/Lighting/Lighting"
#line 7


uniform samplerCube cubeMap;

layout(std140) uniform CPU
{
	float roughness;
	vec3 F0;
	vec3 albedo;
	vec3 CamPos;
	bool hasCubeMap;
};



in vec3 v_Normal;
in vec3 fpos;

layout (location = 0) out vec4 Color;

void main()
{
	vec3 V = normalize (CamPos.xyz-fpos);
	vec3 N = normalize(v_Normal);
	Color = addPBR(fpos, N, V, albedo, roughness, F0, /*SSDO*/ vec3(0));

	
/*
	vec3 R = reflect(V, N);

	vec3 L = normalize(Lights[0].pos.xyz - fpos);
	vec3 H = normalize (V + L);
	float dotLH = clamp (dot (L, H), 0.0, 1.0);
	float dotLH5 = pow (1.0 - dotLH, 5);
	vec3 F = F0 + (1.0 - F0)*(dotLH5);
	Color = mix(Color, textureCube(cubeMap, R), vec4(F, 0.0));*/

	//Color = textureCube(cubeMap, R);

}