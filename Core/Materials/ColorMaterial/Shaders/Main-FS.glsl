#version 430

#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Lighting/Lighting"
#line 6 



layout(std140) uniform CPU
{	
	vec4 CPU_color;
};


in vec3 v_Normal;
in vec3 fpos;

layout (location = 0) out vec4 Color;

void main()
{
	Color = addPhong(fpos,normalize(v_Normal),CPU_color,CPU_color,vec4(1.0),vec4(0.4,0.6,0.4,128.0));
}