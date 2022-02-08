#version 430

#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Common"
#line 6 

layout(std140) uniform CPU
{
	mat4 MVP;
	mat4 Model;
};

 out gl_PerVertex {
        vec4 gl_Position;
        float gl_PointSize;
        float gl_ClipDistance[];
    };
layout (location = 0) in vec3 Position;
layout (location = 2) in vec3 Normal;

out vec4 positionTest;
void main()
{
	gl_Position = ViewProj * Model * vec4(Position,1.0);
}
