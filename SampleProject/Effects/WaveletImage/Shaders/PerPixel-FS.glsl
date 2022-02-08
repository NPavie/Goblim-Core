#version 420


uniform sampler2D smp_FBO_in;


in vec3 texCoord;

layout (location = 0) out vec4 Color;

void main()
{
	Color = texture(smp_FBO_in, texCoord.xy);
}
