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

out vec3 v_Normal;
out vec3 fpos;

void main()
{
	gl_Position = ViewProj * Model * vec4(Position,1.0);
	//gl_Position = MVP * vec4(Position,1.0);
 	fpos = (Model * vec4(Position,1.0)).xyz;
 	v_Normal = (Model * vec4(Normal,0.0)).xyz;

}
