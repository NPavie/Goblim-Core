#version 430

#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/SpotNoise"
#line 6 


uniform sampler2D smp_FBO_in;

layout(std140) uniform FP_UBO{
	int distribID;
};


in vec3 texCoord;

layout (location = 0) out vec4 Color;

void main()
{

	//Color = vec4(poissonProcessProfile(texCoord.xy * 8.0f, 8 ));
	//Color = vec4( NonUniformDistributionProfileWithKroenecker(texCoord.xy * 8.0f, distribID, 8 ));
	Color = vec4( NonUniformControledDistributionProfile(texCoord.xy * 8.0f, distribID, 4 ));
}
