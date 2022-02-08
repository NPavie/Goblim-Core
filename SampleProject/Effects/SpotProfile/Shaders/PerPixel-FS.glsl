#version 430

#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/SpotNoise"
#line 6 


uniform sampler2D smp_FBO_in;

layout(std140) uniform FP_UBO{
	int dataID;
	int isDistrib;
	int isSpot;
};

in vec3 texCoord;

layout (location = 0) out vec4 Color;

void main()
{
	if(isDistrib == 1) {
		if(isSpot == 1) {
			Color = vec4(spotResponse(vec3(texCoord.xy - 0.5,0.0), dataID));
		}else{
			CachedDistribution dist = fromDistributionBuffer(distrib[dataID]);
			Color = vec4( controlledDistributionProfile(texCoord.xy, dist));
		}
	}
	else Color = 0.5f + 0.5f * vec4(spotResponse(vec3(texCoord.xy - 0.5,0.0), dataID));
}
