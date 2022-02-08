#version 420
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


out vec2 texCoords;
out vec3 fpos;

void main(void)
{
	gl_Position = vec4(Position,1.0);
 fpos = Position;
texCoords = 0.5*(Position.xy+1.0);

}