#version 430
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Common"
#line 5

uniform sampler2D resourceSampler;

layout(std140) uniform CPU
{
	int isUIntDepth;
};



in vec3 texCoord;
layout (location = 0) out vec4 Color;


void main()
{

	
	if(isUIntDepth == 1)
	{	
		// Depth texture in DEPTH_COMPONENT_24
		float depth =  texture(resourceSampler,texCoord.xy).r;
		Color = vec4( depth,0.0,0.0,1.0);
		//Color = vec4( uintBitsToFloat(depthColor.x)   );
	}
	else
	{
		Color = texture(resourceSampler,texCoord.xy);
	}

	vec2 edge = step(vec2(0.49),abs(texCoord.xy-0.5));
	float edgeval = min(edge.x+edge.y,1.0);
	Color = edgeval*vec4(1.0) + (1.0 - edgeval)*Color;

}