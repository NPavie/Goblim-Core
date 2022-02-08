#version 430


uniform layout(binding = 0,size1x16) iimage2D res;
in vec3 texCoord;
layout (location = 0) out vec4 Color;

void main()
{

	//Color = imageLoad(resource,ivec3(texCoord.xy*imageSize(resource).xy,5));
	int val = imageLoad(res,ivec2(texCoord.xy*imageSize(res).xy));

	vec2 edge = step(vec2(0.49),abs(texCoord.xy-0.5));
	float edgeval = min(edge.x+edge.y,1.0);
	Color = edgeval*vec4(1.0) + (1.0 - edgeval)*Color;
	Color.x = 0.5;
	if (val >= 1)
		Color.y = 1.0;
}