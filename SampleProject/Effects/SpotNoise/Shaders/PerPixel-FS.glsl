#version 430

#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/SpotNoise"
#line 6 

uniform sampler2D smp_FBO_in;
uniform sampler2D smp_dataField;

layout(std140) uniform FP_UBO{	
	int nbSpotPerAxis;
	int distribID;
	int noiseType;
};

in vec3 texCoord;

layout (location = 0) out vec4 Color;

void main()
{
	vec2 evaluatedPos = texCoord.xy * 8.0f;

	// Version uniform
	//Color = vec4(spotNoise2D(texCoord.xy * 8.0f, 0, 4 ));

	// Version non-uniform
	//Color = vec4(NonUniformSpotNoiseWithKroenecker(texCoord.xy * 8.0f, 0, 1, 8));
	//vec4 background = vec4(165.0,153,147,255.0) / 255.0;
	vec4 background = vec4(0.0);
	int totalSpots = nbSpotPerAxis * nbSpotPerAxis;


	if(noiseType < 1){
		Color = NonUniformControlledRandomizedSpotNoise(evaluatedPos, 0, totalSpots, distribID);
	} else if(noiseType < 2){
		Color = NonUniformControlledOrderedSpotNoise(evaluatedPos, 0, nbSpotPerAxis, distribID);
	} else if(noiseType < 3){
		//Color = NonUniformControlledRandomImpulseSpotNoise(evaluatedPos, 0, totalSpots, distribID, smp_dataField);
		Color = NonUniformControlledRandomImpulseMultiSpotNoise(evaluatedPos, 0, totalSpots, distribID, smp_dataField);
	} else {
		Color = LocallyControledSpotNoise(
			evaluatedPos, 
			0, 
			totalSpots, 
			distribID, 
			smp_dataField, 
			0.125f );
	}

	//Color = Color.a > 1.0 ? Color / Color.a : Color;
	//Color.rgb = Color.a > 0.0 ? Color.rgb / Color.a : Color.rgb;

	Color = mix(background,Color,Color.a);
	
	
}
