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
layout (location = 3) in vec3 Texture_Coordinates;

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

layout(std140) uniform CPU
{
	mat4 CPU_MVP;
	float f_Grid_height;
	int CPU_numFirstInstance;
};

	
uniform sampler2DArray smp_densityMaps;
uniform sampler2DArray smp_scaleMaps;
uniform sampler2DArray smp_distributionMaps;
uniform sampler2DArray smp_surfaceData;

struct kernel{
	// Changement d'agencement pour un tableau de vec4: 
	vec4 data[7];
	// Index of data :
	// 0 -- ModelData : isActive, model_texIDs_begin, model_texIDs_end
	// 1 -- kernelShape : rx, ry, rz, power
	// 2 -- distributionData : dMax, subdivision, densityPerCell, distribID
	// 3 -- ControlMapsIds : density_texID, distribution_texID, scale_texID, colorMap_texId
	// 4 -- ControlMapsInfluence : densityMap_influence, distributionMap_influence, scaleMap_influence, colorMap_influence
	// 5 -- lightData : attenuation, emittance, reflectance, volumetric obscurance factor
	// 6 -- ModelShape : rx , ry , rz, scale

};

layout(std430,binding=3) readonly buffer KernelBuffer
{
	ivec4 nbKernel; //.x
	kernel Kernels[];
};

struct Voxel
{
	vec4 position;
	vec4 normale;
	vec4 tangente;
	vec4 coord_texture;
	vec4 coord_surface; // UVW de l'élément de surface associé + distance à cet élément de surface
};

layout(std430, binding=4) readonly buffer VoxelBuffer
{
	vec4 size;
	Voxel Voxels[];
};

vec3 rotateAround(vec3 vec, float angleInRadians, vec3 aroundAxe) // from GLM matrix_transform.inl
{
	float cosA = cos(angleInRadians);
	float sinA = sin(angleInRadians);

	vec3 axis = normalize(aroundAxe);
	vec3 temp = (1.0 - cosA) * axis;
	mat3 rotationMatrix;
	rotationMatrix[0][0] = cosA + temp[0] * axis[0];
	rotationMatrix[0][1] = 0 	+ temp[0] * axis[1] + sinA * axis[2];
	rotationMatrix[0][2] = 0 	+ temp[0] * axis[2] - sinA * axis[1];

	rotationMatrix[1][0] = 0 	+ temp[1] * axis[0] - sinA * axis[2];
	rotationMatrix[1][1] = cosA + temp[1] * axis[1];
	rotationMatrix[1][2] = 0 	+ temp[1] * axis[2] + sinA * axis[0];

	rotationMatrix[2][0] = 0 	+ temp[2] * axis[0] + sinA * axis[1];
	rotationMatrix[2][1] = 0 	+ temp[2] * axis[1] - sinA * axis[0];
	rotationMatrix[2][2] = cosA + temp[2] * axis[2];

	vec3 result = rotationMatrix * vec;
	return normalize(result);

}

mat4 createKernelSpace(
	kernel element, 
	vec3 position, 
	vec3 xDirBase,
	vec3 yDirBase, 
	vec3 wantedDirInTangentSpace, 
	float scale
	)
{
	
	
	// Espace de transformation de l'espace du noyau dans l'espace objet (espace locale élémentaire)
	mat4 kernelSpace = mat4(1.0);
	kernelSpace[0].xyz = xDirBase;
	kernelSpace[1].xyz = yDirBase;
	kernelSpace[2].xyz = normalize(cross(kernelSpace[0].xyz,kernelSpace[1].xyz));
	kernelSpace[3].xyz = position;
	kernelSpace[3].w = 1.0;
	
	// Pour le moment je suppose que le xDirBase et yDirBase passer en parametres sont la tangente et la normale
	mat3 TBN = mat3(xDirBase,kernelSpace[2].xyz,yDirBase);

	vec3 newDirection = wantedDirInTangentSpace.xyz;
	newDirection = (2.0*newDirection ) - 1.0 ; // renormalization
	vec3 normalFromTSpace = normalize(TBN * newDirection); // repassage en objet
	vec3 dirFromTSpace = normalize(xDirBase);
	dirFromTSpace = length(newDirection.xy) > 0.0 ? normalize(TBN * vec3(newDirection.xy,0) ) : dirFromTSpace;

	// calcul des angles de rotations dans la carte
	float theta = acos(dot(xDirBase,dirFromTSpace));
	float phi = acos(dot(yDirBase,normalFromTSpace));
	
	kernelSpace[0].xyz = rotateAround(kernelSpace[0].xyz,theta,kernelSpace[1].xyz);
	kernelSpace[1].xyz = rotateAround(kernelSpace[1].xyz,-phi,kernelSpace[0].xyz);
	kernelSpace[2].xyz = normalize(cross(kernelSpace[0].xyz,kernelSpace[1].xyz));
	// */

	// rescaling
	vec3 kernelScale = element.data[6].xyz * (element.data[6].w * scale);
	kernelSpace[0].xyz *= kernelScale.x;
	kernelSpace[1].xyz *= kernelScale.y;
	kernelSpace[2].xyz *= kernelScale.z;
	
	return kernelSpace;
}


out vec4 v_position;
out vec3 v_normal;
out vec3 v_textureCoords;
out vec3 surface_position;
out vec3 surface_normale;
out mat4 kernelToObjectSpace;
flat out int kernelTextureID;
flat out int v_invalidKernel;

out kernel computeKernel;

out Voxel rendered;

out float gridHeight;

out vec4 kernelBaseColor;


void main()
{

	// Instancing de voxel de distributioon : 
	// Pour chaque voxel instancié, on extrude le voxel de shellHeight * voxel (rasterization)
	// et on passe au fragment shader pour évaluation par splatting des noyaux dans le voxel

	// Pour chaque instance d'un quad
	// - On crée un espace d'évaluation de noyau sur la surface
	// - On en déduit la nouvelle MVP du quad : Proj * View * Model * SpaceInObject
	// - On projette le quad

	gridHeight = f_Grid_height;
	int realInstanceId = gl_InstanceID;
	
	v_invalidKernel = 0;
	computeKernel = Kernels[0];
	rendered = Voxels[realInstanceId];

	// Extrusion de Voxel 
	vec3 voxel_vertexPosition = rendered.position.xyz + ((Position-vec3(0.5))*2.0 * size.y) ;
	
	
	//gl_Position = CPU_MVP * vec4(Position,1.0);
	gl_Position = CPU_MVP * vec4(voxel_vertexPosition,1.0);
	
}

/*


// Calculs : 
	// total des noyaux = nbLignes * nbColonnes * nbNoyauxParCellule
	// num du noyau = instance % nbNoyauxParCellule
	// instanceCell = instance / nbNoyauxParCellule (en division en entière)

	int nbCellPerLine = int(computeKernel.data[2].y);
	int kernelPerCell = int(computeKernel.data[2].z);
	
	// index of kernel and cell
	int numKernel = realInstanceId % kernelPerCell;
	int Cell1DId = realInstanceId / kernelPerCell;

	ivec2 instanceCell;
	instanceCell.x = Cell1DId % nbCellPerLine;
	instanceCell.y = Cell1DId / nbCellPerLine;

	initRandom(instanceCell);

	// random variable
	vec2 randomPosition = vec2(0.0);
	float density;
	float scale;
	float chanceOfPoping;
	float theta;
	float phi;
	// Shifting the seed to he correct position
	for(int i = 0; i < numKernel - 1 ; i++)
	{
		// shift de la seed pour obtenir le bon état du prng
		randomPosition = randomVec2();// cellCenter + clamp(randomIn(0,d_max) * normalize(randomIn(vec2(-1),vec2(1))), vec2(-1), vec2(1));
		theta = random();
		phi = random();
		density = random();// kernels[i].data[4].x * densityMap.x + (1.0 -  kernels[i].data[4].x) * randomIn(0.0,1.0);
		scale = random();// kernels[i].data[4].z * scaleMap.x + (1.0 -  kernels[i].data[4].z) * randomIn(0.0,1.0);
		kernelTextureID =  int(random()) ; //int(randomIn(kernels[i].data[0].y, kernels[i].data[0].z + 0.9));
		chanceOfPoping = random();
	}

	// Begin : Génération du noyau
	
	//kernelBaseColor = vec4(normalize(randomVec3()),1.0);
	// Création de l'espace d'évaluation dans l'espace objet
	// Position aléatoire : random dans la cellule
	randomPosition = randomInFrame( vec2(instanceCell) + 0.5, vec2(0.0), vec2(0.5) );
	randomPosition = clamp(randomPosition / vec2(nbCellPerLine),vec2(0.001), vec2(0.999));

	
	// Surface attributes at this position
	vec4 positionMap = textureLod(smp_surfaceData,vec3(randomPosition,0), 0);
	vec3 normalMap = normalize( textureLod(smp_surfaceData,vec3(randomPosition,1), 0).xyz );
	vec3 tangentMap = normalize( textureLod(smp_surfaceData,vec3(randomPosition,2), 0).xyz );
	vec3 biTangent = normalize(cross(normalMap,tangentMap));
	
	vec4 densityMap = vec4(0);
	vec4 distributionMap = vec4(0,0,1,0);
	vec4 scaleMap = vec4(0);
	
	// Carte de controle
	densityMap = textureLod(smp_densityMaps,vec3(randomPosition,int(computeKernel.data[3].x)),0);
	distributionMap = textureLod(smp_distributionMaps,vec3(randomPosition,int(computeKernel.data[3].y)), 0);
	scaleMap = textureLod(smp_scaleMaps, vec3(randomPosition,int(computeKernel.data[3].z)),0);



	// Direction random selon theta et phi
	theta = randomIn(-PI, PI);
	phi = randomIn(0, PI_4);
	vec3 randomDir = normalize(vec3(sin(theta) * cos(phi),sin(theta) * sin(phi),cos(theta)));
	// Repasser random dir en espace tangent
	randomDir = (randomDir + 1.0) * 0.5;

	vec3 wantedDir = distributionMap.xyz * computeKernel.data[4].y + (1.0 - computeKernel.data[4].y) * randomDir;

	density =  computeKernel.data[4].x * densityMap.x + (1.0 -  computeKernel.data[4].x) * randomIn(0.0,1.0);
	scale =  computeKernel.data[4].z * scaleMap.x + (1.0 -  computeKernel.data[4].z) * randomIn(0.0,1.0);
	

	kernelTextureID = int(randomIn(computeKernel.data[0].y, computeKernel.data[0].z + 0.9));
	chanceOfPoping = random();

	kernelToObjectSpace = mat4(1.0);
	kernelToObjectSpace = createKernelSpace(
								computeKernel, 
								positionMap.xyz, 
								tangentMap,
								normalMap, 
								wantedDir, 
								scale * f_Grid_height
								);

	v_normal = normalMap;
	v_invalidKernel = (positionMap.w == 1.0 && density > 0.0 && chanceOfPoping <= density && scale > 0.0) ? 0 : 1;
	v_position = (CPU_MVP * kernelToObjectSpace * vec4(Position.x, Position.y,Position.z ,1.0));
	gl_Position = v_position;
	v_textureCoords = (Texture_Coordinates - vec3(0.5)) * 2.0f; // Pour mettre les UV entre -1 et 1 (comme les vertex du quad)
	surface_position = positionMap.xyz;
	surface_normale = normalMap.xyz;

*/
