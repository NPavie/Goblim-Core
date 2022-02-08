#version 440 core
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Lighting/Lighting"
#line 5

uniform sampler2DArray GBuffers;
in vec3 texCoord;


layout(std140) uniform CPU
{	
	vec3 CamPos;
};

layout (location = 0) out vec3 Color;

void main()
{
	//----- Ambient Occlusion -----//
	float AO = textureLod(GBuffers, vec3(texCoord.xy, 0.0), 0).r;
	//----- Albedo : Diffuse -----//
	vec3 Diffuse = textureLod(GBuffers, vec3(texCoord.xy, 1.0), 0).rgb;
	//----- Specular -----//
	vec3 Specular = textureLod(GBuffers, vec3(texCoord.xy, 2.0), 0).rgb;
	//----- Position -----//
	vec3 Position = textureLod(GBuffers, vec3(texCoord.xy, 3.0), 0).rgb;
	//----- Normal -----//
	vec3 N = textureLod(GBuffers, vec3(texCoord.xy, 4.0), 0).rgb;
	
	vec3 T, B;
	T = vec3(1.0, 0.0, 0.0);
	T = normalize(T - dot(T, N) * N);
	B = cross(N, T);

	mat3 TBN = transpose(mat3(T, B, N));

	//Color = Shading(T, B, N, normalize(CamPos-Position), Position, AO, Diffuse, Specular, vec4(1.0, 15.0, 16.0, 16.0), 1);


	vec3 baseColor = Diffuse;
	float reflectance = 0.5;
	float metalMask = 1.0;
	vec3 F0 = 0.16f*(reflectance*reflectance)*(1.0f - metalMask) + baseColor*metalMask;


	
	//Color = textureLod(GBuffers, vec3(texCoord.xy, 3.0), 0).rgb;
	Color = addPBR( Position, N, normalize (CamPos.xyz-Position), baseColor, 0.3, F0).rgb;
	
//	Color = abs(N);
	/*if(Position != vec3(1.0))
		Color = addPBR( Position, N, baseColor, 0.3, F0).rgb;
		else Color = vec3(1,0,0);*/
	//Color = texture(test, texCoord.xy).rgb;

}