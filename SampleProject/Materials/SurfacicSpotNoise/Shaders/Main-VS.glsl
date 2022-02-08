#version 420

/* Pour utiliser le buffer des matrices de la caméra
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Common"
*/
#line 8

layout(std140) uniform CPU
{
	mat4 CPU_modelViewMatrix;
};

layout(location = 0) in vec3 Position;
layout(location = 2) in vec3 Normal;
layout(location = 3) in vec3 Texture_Coordinates;
layout(location = 4) in vec3 Tangent;

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};


out Vertex{
	vec3 Position;
	vec3 Normal;
	vec3 Texture_Coordinates;
	vec3 Tangent;
}v_out;


void main(){

	// gl_position obligatoire pour rasterization
	gl_Position = CPU_modelViewMatrix * vec4(Position,1.0);

	v_out.Position = gl_Position.xyz;

	v_out.Normal = Normal;
	v_out.Texture_Coordinates = Texture_Coordinates;
	v_out.Tangent = Tangent;
	
	
	
}
