/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#version 420

layout(std140) uniform CPU
{
	mat4 MVP;
};

out gl_PerVertex 
{
	vec4 gl_Position;
	float gl_PointSize;
	float gl_ClipDistance[];
};

layout (location = 0) in vec3 Position;

out vec2 texCoord;

void main()
{
	texCoord = 0.5 * (Position.xy + 1.0);
	gl_Position = vec4(Position,1.0);
}