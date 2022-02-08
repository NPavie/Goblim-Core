#version 420

layout(std140) uniform CPU
{	

	vec4 color;
};


layout (location = 0) out vec4 Color;
void main()
{
Color = color;
}