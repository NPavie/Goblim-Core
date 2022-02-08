#version 430
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Lighting/Lighting"
#line 5 

uniform sampler2D baseColorTex; 
uniform sampler2D aoTex;
uniform sampler2D metalMaskTex;
uniform sampler2D normalTex; 
uniform sampler2D reflectanceTex;
uniform sampler2D roughnessTex;

layout(std140) uniform CPU
{	

//Si utilisation de textures bindless
/*sampler2D baseColorTex; 
	sampler2D aoTex;
	sampler2D metalMaskTex;
	sampler2D normalTex; 
	sampler2D reflectanceTex;
	sampler2D roughnessTex;
*/
	bool use_ao;
	bool use_metal_mask;
	bool use_normal;
	bool use_base_color;
	bool use_roughness;
	bool use_reflectance;

	// Si on n'utilise pas de textures :
	vec3 metalMask_Reflectance_Roughness;
	vec3 base_Color;


	bool use_height;
	bool use_selfshadow;
	vec3 CamPos;
	float bumpScale; // Bumb Scale : Parallax Depth
	float MaxQuality; // Maximum Number of Steps : Quality of Raymarching	

	
};

// Si Geometry Shader
//in vec3 g_T;
//in vec3 g_B;
//in vec3 g_N;
//in vec3 g_Position;
//in vec3 g_TexCoord;
//in mat3 g_TBN;
//in vec4 g_ScreenPosition;
//in vec4 g_ScreenLastPosition;
//in vec4 g_PositionInCamera;

in vec3 v_T;
in vec3 v_B;
in vec3 v_N;
in vec3 v_Position;
in vec3 v_TexCoord;
in mat3 v_TBN;
in vec4 v_ScreenPosition;
in vec4 v_ScreenLastPosition;
in vec4 v_PositionInCamera;




layout (location = 0) out vec3 AOColor;
layout (location = 1) out vec3 BaseColor;
layout (location = 2) out vec3 MetalMask_Reflectance_Roughness;
layout (location = 3) out vec4 WorldPosition;
layout (location = 4) out vec3 WorldNormal;
layout (location = 5) out vec2 ScreenVelocity;
//layout (location = 6) out vec3 depthInViewSpace;

vec4 ParallaxOcclusionMapping(in float ParallaxDepth, in float StartDepth, in float StopDepth, in float NumSteps, in vec2 TexCoords, in vec3 V)
{
	float StepHeight = abs(StartDepth - StopDepth) / NumSteps; // Calcul de la profondeur parcourue à chaque itération

	vec2 SearchHeight = vec2(0.0); // Profondeur du point final : profondeur exacte entre SearchHeight.x et SearchHeight.y
	vec2 OffsetCoords = TexCoords; // Coordonées du point final

	vec2 Offset = vec2(StepHeight * ParallaxDepth * V.xy); // Facteur de déplacement dans la texture à chaque itération

	float CurrentDepth = StartDepth; // Profondeur de départ
	float Depth; // Profondeur courante

	for(float i = 0.0; i < NumSteps; ++i)
	{
		CurrentDepth -= StepHeight; // Décrémentation de la profondeur courante
		OffsetCoords += Offset; // Déplacement des coordonées de texture
		Depth = texture2D(normalTex, OffsetCoords).a; // Calcul de la profondeur aux nouvelles coordonées
		
		if(CurrentDepth <= Depth) // Si la nouvelle profondeur est inférieur ou égale à l'ancienne
			break; // Fin de boucle
	}

	OffsetCoords -= Offset; // Retour au dernières bonnes coordonées connues
	SearchHeight.x = CurrentDepth + StepHeight; // Retour à la dernière bonne profondeur connue
	SearchHeight.y = CurrentDepth; // Profondeur actuelle

	return vec4(OffsetCoords, SearchHeight); // Retour des dernières bonnes coordonées connues et de l'intervalle de profondeur dans lequel se trouve l'intersection
}

vec4 ReliefMapping(in float ParallaxDepth, in float StartDepth, in float StopDepth, in float NumSteps, in vec2 TexCoords, in vec3 V)
{
	vec4 POMResult = ParallaxOcclusionMapping(ParallaxDepth, StartDepth, StopDepth, NumSteps, TexCoords, V); // Calcul de l'intervalle dans lequel se trouve l'intersection
	return ParallaxOcclusionMapping(ParallaxDepth, POMResult.z, POMResult.w, NumSteps, POMResult.xy, V); // Calcul de la position de l'intersection dans l'intervalle
}

float SelfShadow(in float ParallaxDepth, in float StartDepth, in float StopDepth, in float NumSteps, in vec2 TexCoords, in float ShadowPower, in vec3 L)
{
	float StepHeight = abs(StartDepth - StopDepth) / NumSteps; // Calcul de la profondeur parcourue à chaque itération

	vec2 OffsetCoords = TexCoords; // Coordonées du point final

	vec2 Offset = vec2(StepHeight * ParallaxDepth * L.xy); // Facteur de déplacement dans la texture à chaque itération

	float CurrentDepth = StartDepth; // Profondeur de départ
	float Depth; // Profondeur courante

	float NumSubSurfaceSamples = 0.0;

	for(float i = 0.0; i < NumSteps; ++i)
	{
		CurrentDepth += StepHeight; // Décrémentation de la profondeur courante
		OffsetCoords -= Offset; // Déplacement des coordonées de texture
		Depth = texture2D(normalTex, OffsetCoords).a; // Calcul de la profondeur aux nouvelles coordonées
		if(CurrentDepth >= Depth) // Si la nouvelle profondeur est inférieur ou égale à l'ancienne
			NumSubSurfaceSamples += ShadowPower;
	}

	return (NumSubSurfaceSamples / NumSteps);
}

void main()
{	
	vec4 offsetCoord = vec4(v_TexCoord.xy, 0.0, 0.0);

	//------------------- PARALLAX MAPPING -------------------//
	if(use_height)
	{	
		vec3 V, ViewDir = normalize(CamPos - v_Position);
		V.x  = dot(v_T, ViewDir);
		V.y  = dot(v_B, ViewDir);
		V.z  = dot(v_N, ViewDir);

		offsetCoord = ReliefMapping(bumpScale, 1.0, 0.0, MaxQuality, v_TexCoord.xy, V);
		//if(offsetCoord.x < 0.0 || offsetCoord.x > 1.0 || offsetCoord.y < 0.0 || offsetCoord.y > 1.0)
		//	discard;
	}

	//------------------- CLASSIC TEXTURING -------------------//
	if(use_base_color) {
		vec4 tmp = texture(baseColorTex, offsetCoord.xy);
		if(tmp.a < 0.25)
			discard;
		BaseColor = pow(tmp.rgb, vec3(2.2));
	}
	else BaseColor = base_Color;


	if (use_ao) // If the AO Map is set
		AOColor = texture(aoTex, offsetCoord.xy).rgb;
	else
		AOColor = vec3(1.0);
		
	
	if(use_metal_mask) { // If the Metal Map is set
		MetalMask_Reflectance_Roughness.r = texture(metalMaskTex, offsetCoord.xy).r;
	}
	else MetalMask_Reflectance_Roughness.r = metalMask_Reflectance_Roughness.r;

	if(use_reflectance) { // If the Reflectance Map is set
		MetalMask_Reflectance_Roughness.g = texture(reflectanceTex, offsetCoord.xy).r;
	}
	else MetalMask_Reflectance_Roughness.g = metalMask_Reflectance_Roughness.g;

	if(use_roughness) { // If the Roughness Map is set
		MetalMask_Reflectance_Roughness.b = texture(roughnessTex, offsetCoord.xy).r;
	}
	else MetalMask_Reflectance_Roughness.b = metalMask_Reflectance_Roughness.b;



	if (use_normal) {	// If the Normal Map is set
		WorldNormal = normalize(inverse(v_TBN) * normalize(texture(normalTex, offsetCoord.xy) * 2.0 - 1.0).xyz);
	}
	else
		WorldNormal = normalize(v_N);
		
	
	WorldPosition = vec4(v_Position, 1.0);

	// Compute screen velocity for Per Object Motion Blur
	vec2 a = (v_ScreenPosition.xy / v_ScreenPosition.w) * 0.5 + 0.5;
    vec2 b = (v_ScreenLastPosition.xy / v_ScreenLastPosition.w) * 0.5 + 0.5;
    ScreenVelocity = (a - b);

	//------------------- SELF SHADOWS -------------------//
	if(use_selfshadow)
	{
		float Shadow;
		vec3 L, LightDir;

		for (int i = 0; i < nbLights.x; i++)
		{
			LightDir = normalize(Lights[i].pos.xyz - v_Position);
			L.x  = dot(v_T, LightDir);
			L.y  = dot(v_B, LightDir);
			L.z  = dot(v_N, LightDir);

			Shadow = SelfShadow(bumpScale, offsetCoord.z, 1.0, MaxQuality, offsetCoord.xy, 1.0, L);
	
			AOColor *= Shadow;
		}
	}
	//depthInViewSpace = vec3(-v_PositionInCamera.z,0,1);
	//AOColor = vec3(0, 1, 1);
}