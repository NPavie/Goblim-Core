#version 420


uniform sampler2D fboIn;
in vec3 texCoord;
layout (location = 0) out vec4 Color;
void main()
{

vec4 colBase = texture(fboIn,texCoord.xy); 
	Color =  colBase;
}
