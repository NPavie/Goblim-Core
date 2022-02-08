#version 430
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Common"
#include "/Materials/Common/Random/Random"				// Pseudo Random Number Generator						
#line 6

// constantes
#ifndef PI 
	#define PI 3.14159265359
	#define PI_2 1.57079632679
	#define PI_4 0.78539816339
#endif


layout (location = 0) in vec3 Position;

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

layout(std140) uniform CPU
{
	mat4 CPU_MVP;
	float CPU_voxelSize;
};

struct VoxelData
{
	vec4 position;
	vec4 normale;
	vec4 tangente;
	vec4 coord_texture;
	vec4 coord_surface; // UVW de l'élément de surface associé + distance à cet élément de surface
};

layout(std430,binding=4) readonly buffer VoxelBuffer
{
	ivec4 size;
	VoxelData grid[];
};


out vec4 v_position;


void main()
{
	int cubeId = gl_InstanceID;

	// Pour le cube unitaire

	// Pipeline du rendu volumetric :
	// Rasterization standard
	v_position = CPU_MVP * vec4( grid[cubeId].position.xyz + ((Position - 0.5) * CPU_voxelSize)  ,1.0);
	//v_position = CPU_MVP * vec4( (Position - 0.5)  ,1.0);
	gl_Position = v_position;


}
