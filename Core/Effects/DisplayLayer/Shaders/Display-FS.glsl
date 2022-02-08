#version 420

layout(std140) uniform CPU {
	int layer_bound;
};

uniform sampler2DArray resource;
in vec3 texCoord;
layout (location = 0) out vec4 Color;

void main()
{

	Color = textureLod(resource,vec3(texCoord.xy,layer_bound),0);
	vec2 edge = step(vec2(0.49),abs(texCoord.xy-0.5));
	float edgeval = min(edge.x+edge.y,1.0);
	Color = edgeval*vec4(1.0) + (1.0 - edgeval)*Color;

}