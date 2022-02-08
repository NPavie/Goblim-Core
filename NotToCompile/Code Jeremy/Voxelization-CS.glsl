#version 430
#extension GL_ARB_compute_variable_group_size : enable
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_buffer_object : enable
layout (local_size_x = 512, local_size_y = 1, local_size_z = 1) in;

/*
in uvec3 gl_NumWorkGroups;
in uvec3 gl_WorkGroupID;
in uvec3 gl_LocalInvocationID;
in uvec3 gl_GlobalInvocationID;
in uint  gl_LocalInvocationIndex;
*/

layout(std140) uniform CPU
{
	int offsetDonnee;
};

struct VoxelData
{
	vec4 position;
	vec4 normale;
	vec4 tangente;
	vec4 coord_texture;
	vec4 coord_surface; // UVW de l'élément de surface associé + distance à cet élément de surface
	ivec4 boolean;
};

struct deleteArray
{
	ivec4 del;
};

layout(std430,binding=4) readonly buffer VoxelBuffer
{
	ivec4 size;
	VoxelData grid[];
};

layout(std430,binding=15) buffer flagBuffer
{
	deleteArray dA[];
};


// Compute shader de réduction de voxel
void main()
{
	// Récupérer le voxel à étudier pour l'invocation courante
	uint globalId = gl_GlobalInvocationID.x;
	//int local_index = int(gl_LocalInvocationID.x);
	int s = size.x;
	int end;
	if(globalId < s)
	{
		if(dA[globalId].del.x == 0)
		{
			int begin = 100 * (offsetDonnee-1);
			if(globalId >= begin)
			{
				begin = int(globalId);
			}
			vec4 v1_coord_surface = grid[globalId].coord_surface;
			vec4 v1_coord_texture = grid[globalId].coord_texture;
			vec4 v2_coord_surface, v2_coord_texture;
			end = 100 * offsetDonnee;
			if(end > s)
				end = s;
			for(int i = begin; i < end; i++)
			{
				if(i != globalId)
				{
					v2_coord_surface = grid[i].coord_surface;
					v2_coord_texture = grid[i].coord_texture;
					// Calcul direct :
					if(v1_coord_texture.xyz == v2_coord_texture.xyz && v1_coord_surface.w >= v2_coord_surface.w)
					{
						dA[globalId].del.x = 1;
						i = end;
					}
				}
			}
		}
	}
}
//if((all(equal(vec3(v1_coord_texture.xyz),vec3(v2_coord_texture.xyz)))) && (v1_coord_surface.w > v2_coord_surface.w))
