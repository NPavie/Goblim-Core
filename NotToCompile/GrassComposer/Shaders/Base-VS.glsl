#version 420



/**
*	Basic vertex shader for FBO rendering
*/

layout(std140) uniform CPU
{
	mat4 MVP;
};

out gl_PerVertex {
        vec4 gl_Position;
        float gl_PointSize;
        float gl_ClipDistance[];
};

layout (location = 0) in vec3 Position;
layout (location = 3) in vec3 TextureCoordinates;


out vec3 texCoord;
void main()
{
	gl_Position = vec4(Position,1.0);
	texCoord = 0.5*(Position.xyz+1.0);

}
