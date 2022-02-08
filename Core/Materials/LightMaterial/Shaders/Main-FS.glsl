#version 410

layout(std140) uniform CPU_FS
{	

	vec4 color;
};


layout (location = 0) out vec4 Color;
void main()
{
    Color = color;
}