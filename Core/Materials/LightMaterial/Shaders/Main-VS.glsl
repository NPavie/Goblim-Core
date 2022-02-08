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


void main()
{
gl_Position = MVP * vec4(Position,1.0);

}
