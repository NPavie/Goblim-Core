#version 430
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Common"
#include "/Materials/Common/Random/Random"				// Pseudo Random Number Generator
#include "/Materials/Common/Kernels"						
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


out vec4 v_position;
out vec3 v_normal;
out vec3 v_textureCoords;
out vec3 surface_position;
out vec3 surface_normale;
out mat4 kernelToObjectSpace;
flat out int kernelTextureID;
flat out int v_invalidKernel;

out kernel computeKernel;
out float gridHeight;

out vec4 kernelBaseColor;

out vec3 v_fragCoord;

void main()
{
	// Pour chaque instance d'un quad
	// - On crée un espace d'évaluation de noyau sur la surface
	// - On en déduit la nouvelle MVP du quad : Proj * View * Model * SpaceInObject
	// - On projette le quad

	gridHeight = f_Grid_height;
	int realInstanceId = gl_InstanceID + CPU_numFirstInstance;
	
	v_invalidKernel = 0;

	computeKernel = Kernels[0];

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
	setRandomSeedForKernelNum(numKernel);
	
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
								(positionMap.xyz + 0.01 * normalize(normalMap.xyz)), 
								tangentMap,
								normalMap, 
								wantedDir, 
								scale * f_Grid_height
								);

	v_normal = normalMap;
	v_invalidKernel = (positionMap.w == 1.0 && density > 0.0 && chanceOfPoping <= density && scale > 0.0) ? 0 : 1;
	// Cube entre 0 et 1 ; passage en -1 1 => (position - 0.5)*2.0
	v_position = vec4( (Position-0.5f)*2.0f , 1.0);
	//v_position = vec4( Position, 1.0);
	v_position = (CPU_MVP * kernelToObjectSpace * v_position);
	gl_Position = v_position;
	//v_fragCoord = v_position;
	v_fragCoord = v_position.xyz / v_position.w;
	v_fragCoord = (v_fragCoord * 0.5) + 0.5;

	//v_textureCoords = (Texture_Coordinates - vec3(0.5)) * 2.0f; // Pour mettre les UV entre -1 et 1 (comme les vertex du quad)
	surface_position = positionMap.xyz;
	surface_normale = normalMap.xyz;
}
