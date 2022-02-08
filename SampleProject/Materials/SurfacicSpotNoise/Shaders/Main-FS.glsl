#version 430


#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Lighting/Lighting"
#include "/Materials/Common/SpotNoise"
#line 6 

uniform sampler2D smp_FBO_in;
uniform sampler2D smp_dataField;

layout(std140) uniform FP_UBO{	
	int nbSpotPerAxis;
	int distribID;
	int noiseType;
	float textureScalingFactor;
	mat4 CPU_modelMatrix;

};

in Vertex{
	vec3 Position;
	vec3 Normal;
	vec3 Texture_Coordinates;
	vec3 Tangent;
}v_in;



layout (location = 0) out vec4 Color;

// For multiple render target
//layout (location = 1) out vec4 Color1;
//layout (location = 2) out vec4 Color2;
//layout (location = 3) out vec4 Color3;
// */

void main()
{
	Color = vec4(0.0);

	vec2 evaluatedPos = v_in.Texture_Coordinates.xy * textureScalingFactor;

	// Version uniform
	//Color = vec4(spotNoise2D(texCoord.xy * 8.0f, 0, 4 ));

	// Version non-uniform
	//Color = vec4(NonUniformSpotNoiseWithKroenecker(texCoord.xy * 8.0f, 0, 1, 8));
	//vec4 background = vec4(165.0,153,147,255.0) / 255.0;
	vec4 background = vec4(0.0);
	int totalSpots = nbSpotPerAxis * nbSpotPerAxis;

	vec4 noiseResult = Color;
	vec4 noise_x = Color;
	vec4 noise_y = Color;

	if(noiseType < 1){
		noiseResult = NonUniformControlledRandomizedSpotNoise(evaluatedPos, 0, totalSpots, distribID);
	} else if(noiseType < 2){
		noiseResult = NonUniformControlledOrderedSpotNoise(evaluatedPos, 0, nbSpotPerAxis, distribID);
	} else if(noiseType < 3) {
		//Color = NonUniformControlledRandomImpulseSpotNoise(evaluatedPos, 0, totalSpots, distribID, smp_dataField);
		noiseResult = NonUniformControlledRandomImpulseMultiSpotNoise(evaluatedPos, 0, totalSpots, distribID, smp_dataField);
		noise_x = NonUniformControlledRandomImpulseMultiSpotNoise(evaluatedPos+vec2(0.05,0.0), 0, totalSpots, distribID, smp_dataField);
		noise_y = NonUniformControlledRandomImpulseMultiSpotNoise(evaluatedPos+vec2(0.0,0.05), 0, totalSpots, distribID, smp_dataField);
	} else {
		noiseResult = LocallyControledSpotNoise(
			evaluatedPos, 
			0, 
			totalSpots, 
			distribID, 
			smp_dataField, 
			0.125f );
		noise_x = LocallyControledSpotNoise(
			evaluatedPos+vec2(0.05,0.0), 
			0, 
			totalSpots, 
			distribID, 
			smp_dataField, 
			0.125f );
		noise_y = LocallyControledSpotNoise(
			evaluatedPos+vec2(0.0,0.05), 
			0, 
			totalSpots, 
			distribID, 
			smp_dataField, 
			0.125f );
	}

	//vec4 noiseLeft = dFdx(noiseResult);
	//vec4 noiseUp = dFdy(noiseResult);
	float height = 2.0 * noiseResult.a;

	float difHeightX = 2.0 * noise_x.a - height;
	float difHeightY = 2.0 * noise_y.a - height;

	vec3 newNormalInTSpace = normalize(vec3(difHeightX,difHeightY,1.0));
	mat3 TSpace = mat3(
		v_in.Tangent, 
		-normalize(cross(v_in.Tangent,v_in.Normal)), 
		v_in.Normal);

	vec3 posInWorld = 
		( CPU_modelMatrix * vec4(v_in.Position + height * v_in.Normal,1.0)
		).xyz;
	//vec3 normalInWorld = normalize((CPU_modelMatrix * vec4(v_in.Normal,0.0)).xyz);
	vec3 normalInWorld = normalize(
		(CPU_modelMatrix * vec4( normalize(TSpace * newNormalInTSpace),0.0)
			).xyz );



	// A tester : au lieu d'une couler, renvoyer une normale et calculer une couleur en fonction de la hauteur

	//if(noiseResult.a < 0.05) discard;
	noiseResult = noiseResult.a > 1.0 ? 
		noiseResult / noiseResult.a : 
		noiseResult;
	//vec4 patternColor = mix(background,noiseResult,noiseResult.a);

	Color = addPhong(
		posInWorld, 
		normalInWorld,
		background, 
		noiseResult,
		vec4(1.0),
		vec4(0.2,0.5,0.1,128.0));

	//Color = vec4(abs(difHeightX),abs(difHeightY),0,1);

	//Color1 = /* Whatever */ ;
	//Color2 = /* Whatever */ ;
	//Color3 = /* Whatever */ ;

	
	// */
}