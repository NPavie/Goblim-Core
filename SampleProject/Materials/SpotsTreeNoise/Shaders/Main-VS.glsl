#version 430

#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Common"
#line 6 

/* Imported From Common :
mat4 ViewProj;
*/

layout(std140) uniform CPU
{
	//mat4 MVP;
	mat4 Model;
};

 out gl_PerVertex {
        vec4 gl_Position;
        float gl_PointSize;
        float gl_ClipDistance[];
    };
layout (location = 0) in vec3 Position;
layout (location = 2) in vec3 Normal;
layout (location = 3) in vec3 Texture_Coordinates;

out vec3 worldNormal;
out vec3 worldPosition;
out vec3 textureCoordinates;

void main()
{
	gl_Position = ViewProj * Model * vec4(Position,1.0);
 	worldPosition = (Model * vec4(Position,1.0)).xyz;
 	worldNormal = (Model * vec4(Normal,0.0)).xyz;
 	textureCoordinates = Texture_Coordinates;
}
