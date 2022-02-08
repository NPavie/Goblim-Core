#version 430
#extension GL_ARB_compute_variable_group_size : enable
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_buffer_object : enable
layout (local_size_x = 128, local_size_y = 1, local_size_z = 1) in;

/*in uvec3 gl_NumWorkGroups;
in uvec3 gl_WorkGroupID;
in uvec3 gl_LocalInvocationID;
in uvec3 gl_GlobalInvocationID;
in uint  gl_LocalInvocationIndex;*/

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

layout(std430,binding=6) buffer trianglesBuffer
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
		// Calcul de la grille de voxel utiles
		// Pour chaque prismes, calculer les voxels intersectants le prismes et leur distance à la surface initiale
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

		// Rajout : calcul du prisme et de sa boite englobante
		vec3 se1 = geometries[triangles[globalId].s.x].pointextr.xyz;
		vec3 se2 = geometries[triangles[globalId].s.y].pointextr.xyz;
		vec3 se3 = geometries[triangles[globalId].s.z].pointextr.xyz;

		vec3 prismMin = s1.xyz;
		vec3 prismMax = s1.xyz;

		// calcul d'une boite englobante pour le prisme (espace objet) (min et max de chaque composante de chaque point)
		//prismMin = min(s1,min(s2,min(s3,min(se1,min(se2,se3)))));
		//prismMax = max(s1,max(s2,max(s3,max(se1,max(se2,se3)))));
		for (int c = 0; c < 3; c++)
		{
			// MIN
			if (s1[c] < prismMin[c]) prismMin[c] = s1[c];
			if (se1[c] < prismMin[c]) prismMin[c] = se1[c];

			if (s2[c] < prismMin[c]) prismMin[c] = s2[c];
			if (se2[c] < prismMin[c]) prismMin[c] = se2[c];

			if (s3[c] < prismMin[c]) prismMin[c] = s3[c];
			if (se3[c] < prismMin[c]) prismMin[c] = se3[c];

			// MAX
			if (s1[c] > prismMax[c]) prismMax[c] = s1[c];
			if (se1[c] > prismMax[c]) prismMax[c] = se1[c];

			if (s2[c] > prismMax[c]) prismMax[c] = s2[c];
			if (se2[c] > prismMax[c]) prismMax[c] = se2[c];

			if (s3[c] > prismMax[c]) prismMax[c] = s3[c];
			if (se3[c] > prismMax[c]) prismMax[c] = se3[c];
		}

		// Calcul du min et du max du prism dans le volume englobant subdivisé
		//1ere étape en se rapporte au repère de la boite englobante (minCoord)
		vec3 minPoint = ((prismMin - cube.minCoord.xyz) / (2.0 * cube.halfVector.xyz)) * vec3(float(cube.nbVoxelPerAxis.x));
		vec3 maxPoint = ((prismMax - cube.minCoord.xyz) / (2.0 * cube.halfVector.xyz)) * vec3(float(cube.nbVoxelPerAxis.x));

		// Reclamping dans [0:subdiv[
		minPoint = clamp(floor(minPoint), vec3(0), vec3(cube.nbVoxelPerAxis.x - 1));
		maxPoint = clamp(floor(maxPoint), vec3(0), vec3(cube.nbVoxelPerAxis.x - 1));
			
		triangles[globalId].minPoint = vec4(minPoint, 0.0);
		triangles[globalId].maxPoint = vec4(maxPoint, 0.0);	

	}
}
