#version 420
layout(std140) uniform CPU_FS
{
	float near;
	float far ;
};

in vec3 v_Color;
in float ldepth;

out vec4 Color;

void main()
{
Color = vec4(normalize(v_Color)*0.5+0.5, ldepth);
}