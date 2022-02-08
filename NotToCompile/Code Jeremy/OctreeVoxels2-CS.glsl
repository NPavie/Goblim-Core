#version 430
#extension GL_ARB_compute_variable_group_size : enable
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_buffer_object : enable
layout (local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

struct VoxelData
{
	vec4 position;
	vec4 normale;
	vec4 tangente;
	vec4 coord_texture;
	vec4 coord_surface; // UVW de l'élément de surface associé + distance à cet élément de surface
	ivec4 boolean;
};

struct geometry
{
	vec4 point;
	vec4 normale;
	vec4 tangente;
	vec4 coord;
	vec4 pointextr;
};

struct Triangle
{
	ivec4 s;
	vec4 minPoint;
	vec4 maxPoint;
	ivec4 offsetVoxel;
};

struct Cube
{
	vec4 minCoord;
	vec4 maxCoord;
	vec4 halfVector;
	ivec4 nbVoxelPerAxis;
};

layout(std430,binding=5) readonly buffer geometryBuffer
{
	geometry geometries[];
};

layout(std430,binding=6) readonly buffer trianglesBuffer
{
	ivec4 NBTriangles;
	Cube cube;
	Triangle triangles[];
};

layout(std430,binding=4) writeonly buffer VoxelBuffer
{
	ivec4 size;
	VoxelData grid[];
};

vec3 triangleClosestPoint(in vec3 p, in vec3 a, in vec3 b, in vec3 c, out vec3 barycentriqueCoords)
{
	// Check if P in vertex region outside A
	vec3 ab = b - a;
	vec3 ac = c - a;
	vec3 ap = p - a;
	float d1 = dot(ab, ap);
	float d2 = dot(ac, ap);
	if(d1 <= 0.0f && d2 <= 0.0f)
	{
		barycentriqueCoords = vec3(1, 0, 0);
		return a; // barycentric coordinates (1,0,0)
	}

	// Check if P in vertex region outside B
	vec3 bp = p - b;
	float d3 = dot(ab, bp);
	float d4 = dot(ac, bp);
	if(d3 >= 0.0f && d4 <= d3)
	{
		barycentriqueCoords = vec3(0, 1, 0);
		return b; // barycentric coordinates (0,1,0)
	}

	// Check if P in edge region of AB, if so return projection of P onto AB
	float vc = d1*d4 - d3*d2;
	if(vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f)
	{
		float v = d1 / (d1 - d3);
		barycentriqueCoords = vec3(1.0 - v, v, 0);
		return a + v * ab; // barycentric coordinates (1-v,v,0)
	}
	// Check if P in vertex region outside C
	vec3 cp = p - c;
	float d5 = dot(ab, cp);
	float d6 = dot(ac, cp);
	if(d6 >= 0.0f && d5 <= d6)
	{
		barycentriqueCoords = vec3(0, 0, 1);
		return c; // barycentric coordinates (0,0,1)
	}

	// Check if P in edge region of AC, if so return projection of P onto AC
	float vb = d5*d2 - d1*d6;
	if(vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f)
	{
		float w = d2 / (d2 - d6);
		barycentriqueCoords = vec3(1.0 - w, 0, w);
		return a + w * ac; // barycentric coordinates (1-w,0,w)
	}
	// Check if P in edge region of BC, if so return projection of P onto BC
	float va = d3*d6 - d5*d4;
	if(va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f)
	{
		float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
		barycentriqueCoords = vec3(0, 1.0 - w, w);
		return b + w * (c - b); // barycentric coordinates (0,1-w,w)
	}
	// P inside face region. Compute Q through its barycentric coordinates (u,v,w)
	float denom = 1.0f / (va + vb + vc);
	float v = vb * denom;
	float w = vc * denom;
	float u = 1.0f - v - w;
	barycentriqueCoords = vec3(u, v, w);
	return a + ab * v + ac * w; // = u*a + v*b + w*c, u = va * denom = 1.0f - v - w
}

// Compute shader création grille voxels
void main()
{
	uint globalId = gl_GlobalInvocationID.x;

	if(globalId < NBTriangles.x)
	{
		
		vec3 s1 = geometries[triangles[globalId].s.x].point.xyz;
		vec3 s2 = geometries[triangles[globalId].s.y].point.xyz;
		vec3 s3 = geometries[triangles[globalId].s.z].point.xyz;

		vec3 n1 = geometries[triangles[globalId].s.x].normale.xyz;
		vec3 n2 = geometries[triangles[globalId].s.y].normale.xyz;
		vec3 n3 = geometries[triangles[globalId].s.z].normale.xyz;
	
		vec4 t1 = geometries[triangles[globalId].s.x].tangente;
		vec4 t2 = geometries[triangles[globalId].s.y].tangente;	
		vec4 t3 = geometries[triangles[globalId].s.z].tangente;		

		vec2 u1 = geometries[triangles[globalId].s.x].coord.xy;
		vec2 u2 = geometries[triangles[globalId].s.y].coord.xy;
		vec2 u3 = geometries[triangles[globalId].s.z].coord.xy;


		vec3 coord_texture = vec3(0.0);
		vec3 v_position = vec3(0.0);
		vec3 baryCoords = vec3(0.0);
		vec3 surface_position = vec3(0.0);
		vec3 normale = vec3(0.0);
		float voxelToSurface = 0.0;
		float signToSurface = 0.0;
		vec4 tangente = vec4(0.0);
		vec2 coord_surface = vec2(0.0);

		//on calcule la boite englobante du triangle
		ivec3 minP = ivec3(int(triangles[globalId].minPoint.x), int(triangles[globalId].minPoint.y), int(triangles[globalId].minPoint.z));
		ivec3 maxP = ivec3(int(triangles[globalId].maxPoint.x), int(triangles[globalId].maxPoint.y), int(triangles[globalId].maxPoint.z));

		int offsetVoxel = triangles[globalId].offsetVoxel.x;
		int h = 0;

		//on parcourt les sous voxels de taille 1 de la grille de voxel
		for(int z_i = minP.z; z_i <= maxP.z; z_i++)
		{
			for (int y_i = minP.y; y_i <= maxP.y; y_i++)
			{  
				for (int x_i = minP.x; x_i <= maxP.x; x_i++)
				{
					//Centre du voxel dans l'espace du cube
					coord_texture = (vec3(float(x_i), float(y_i), float(z_i)) + 0.5f) / vec3(float(cube.nbVoxelPerAxis.x));
					//Centre du voxel dans l'espace objet
					v_position = cube.minCoord.xyz + coord_texture * cube.halfVector.xyz * 2.0f;

					//Projection sur la base du prisme
					surface_position = triangleClosestPoint(v_position, s1, s2, s3, baryCoords);
					normale = baryCoords.x * n1 + baryCoords.y * n2 + baryCoords.z * n3;
				
					//Si le voxel est assez proche de la surface, on le garde dans la liste
					voxelToSurface = distance(surface_position, v_position);
					signToSurface = dot(normalize(v_position - surface_position), normalize(normale));
					//if (voxelToSurface <= shellHeight)// && signToSurface >= 0.0)
					//{
					
						tangente = baryCoords.x * t1 + baryCoords.y * t2 + baryCoords.z * t3;
						coord_surface = u1 * baryCoords.x + u2 * baryCoords.y + u3 * baryCoords.z;
						
						VoxelData vd;
						vd.position = vec4(v_position,0);
						vd.normale =  vec4(normale,0);
						vd.tangente =  tangente;
						vd.coord_texture = vec4(coord_texture,0);
						vd.coord_surface = vec4(coord_surface, 0, voxelToSurface);
						grid[offsetVoxel + h] = vd;
						h++;
					//}
				}
			}
		}
	}
}
