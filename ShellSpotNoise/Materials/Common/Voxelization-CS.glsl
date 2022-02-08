#version 430
#extension GL_ARB_compute_variable_group_size : enable
layout (local_size_x = 32, local_size_y = 1, local_size_z = 1) in;

/*
in uvec3 gl_NumWorkGroups;
in uvec3 gl_WorkGroupID;
in uvec3 gl_LocalInvocationID;
in uvec3 gl_GlobalInvocationID;
in uint  gl_LocalInvocationIndex;
*/

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

layout(std430,binding=5) writeonly buffer flagBuffer
{
	int toDelete[];
};


// Compute shader de réduction de voxel
void main()
{
	// Récupérer le voxel à étudier pour l'invocation courante
	int global_index = int(gl_GlobalInvocationID.x);
	//int local_index = int(gl_LocalInvocationID.x);
	vec4 v1_coord_surface = grid[global_index].coord_surface;
	vec4 v1_coord_texture = grid[global_index].coord_texture;
	bool deleteMe = false;
	for(int i = 0; i < size.x; i++)
	{
		if( i == global_index) continue;
		vec4 v2_coord_surface = grid[i].coord_surface;
		vec4 v2_coord_texture = grid[i].coord_texture;
		// Calcul directe :
		deleteMe = (all(equal(v1_coord_texture.xyz,v2_coord_texture.xyz))) && v1_coord_surface.w > v2_coord_surface.w ? true : deleteMe;
	}	

	toDelete[global_index] = deleteMe ? 1 : 0;
}
