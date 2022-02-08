#version 430
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Random/Random"				// Pseudo Random Number Generator
#include "/Materials/Common/Grid"						// Outil de subdivision de l'espace
#include "/Materials/Common/Lighting/Lighting"			// Modèles d'illumination								
#line 7

// constantes
#ifndef PI 
	#define PI 3.14159265359
	#define PI_2 1.57079632679
	#define PI_4 0.78539816339
#endif


// Attributs passer depuis l'étape précédente (et également structure du buffer de triangle pour l'octree)
struct Vertex{
	vec4 Position;
	vec4 Normal;
	vec4 Texture_Coordinates;
};

// autre idée 
// voxel : UV du point le plus proche + distance du voxel 

struct Voxel
{
	vec4 data[3];
	// Index of data :
	// 0 -- min et max des UV 
	// 1 -- distance au maillage en espace objet + direction
	// 2 -- UV du point le plus proche
};

// --------- NOYAUX --------------
struct Kernel{
	vec4 data[7];
	// Index of data :
	// 0 -- ModelData :  isActive, model_texIDs_begin, model_texIDs_end
	// 1 -- kernelShape : rx, ry, scale, kernelPower
	// 2 -- distributionData : distribution_ID(pour distribution différentes par noyau), subdivision, densityPerCell, 
	// 3 -- ControlMapsIds : density_texID, distribution_texID, scale_texID, colorMap_texId
	// 4 -- ControlMapsInfluence : densityMap_influence, distributionMap_influence, scaleMap_influence, colorMap_influence
	// 5 -- lightData : attenuation, emittance, reflectance, volumetric obscurance factor
	// 6 -- ModelShape : rx, ry, rz
};

in Vertex _vertex;


layout(std140) uniform FUBO 
{
	vec2 v2_Screen_size;
	vec3 v3_Object_cameraPosition;
	
	
	mat4 m44_Object_toCamera;
	mat4 m44_Object_toScreen;
	mat4 m44_Object_toWorld;
	
	vec3 v3_AABB_min;
	vec3 v3_AABB_max;
	float f_Grid_height;		
	
	ivec3 iv3_VoxelGrid_subdiv;
	
	// Debug
	bool renderKernels;	
	bool renderGrid;
	float timer;
	
	bool activeShadows;
	bool activeAA;
	float testFactor;
	float dMinFactor;
	float dMaxFactor;
	
	bool modeSlicing;
	
};

//uniform sampler2D smp_ColorTexture;
uniform sampler2D smp_DepthTexture;
uniform sampler2DArray smp_models;	
uniform sampler2DArray smp_densityMaps;
uniform sampler2DArray smp_scaleMaps;
uniform sampler2DArray smp_distributionMaps;
uniform sampler2DArray smp_surfaceData;

uniform sampler3D smp_voxel_UV;
uniform sampler3D smp_voxel_distanceField;

layout (std140) buffer KernelBuffer
{
	int nbModels;	
	Kernel kernels[];
}; 

// Abadon de l'octree, passage aux voxels

layout (std140) buffer VoxelBuffer
{
	Voxel voxels[];
	
};



// ------ Fonctions

int compute1DGridIndex(ivec3 cellPos, ivec3 gridSize)
{
	return cellPos.x + (gridSize.x * cellPos.y) + (gridSize.x*gridSize.y*cellPos.z);
}


//
//	Fonctions nécessaires au splatting
//
/**
*	@brief	projection dans l'espace écran normalisé d'un point
*	@param	point Point 3D à splatter sur l'écran
*	@return position du point à l'écran
*/
vec3 projectPointToScreen(vec3 point,mat4 pointToScreenSpace)
{

	vec4 point_MVP = pointToScreenSpace * vec4(point,1.0);
	vec3 point_NDC = point_MVP.xyz / point_MVP.w;
	// calcul des coordonnées dans un pseudo espace écran ( passage de [-1;1] a [0;1])
	vec3 pointScreen;
	pointScreen = 0.5 + point_NDC * 0.5; 

	return pointScreen;
}


/**
*	@brief	Projection d'une coupe d'un espace 3D dans l'espace écran
*	@param	point Origine du plan de coupe
*	@param  
*	@return position du point à l'écran
*/
mat3 project2DSpaceToScreen(vec3 point, vec3 slice_x, vec3 slice_y, mat4 planeToScreenSpace)
{
	
	vec2 PScreen = projectPointToScreen(point,planeToScreenSpace).xy;
	vec2 XScreen = projectPointToScreen(point + slice_x, planeToScreenSpace).xy;
	vec2 YScreen = projectPointToScreen(point + slice_y, planeToScreenSpace).xy;
	
	// nouveaux axes dans l'espace ecran (pas forcement perpendiculaire)
	vec2 axeX = XScreen - PScreen;
	vec2 axeY = YScreen - PScreen;

	// test
	mat3 XYScreenSpace = mat3(	vec3(axeX,0),
								vec3(axeY,0),
								vec3(PScreen,1));
	

	return XYScreenSpace;
}
// ---------------------------------------------------------------------

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

float formeFunction(vec3 v, vec3 r)
{
	return ( (pow(v.x,2)/pow(r.x,2)) + (pow(v.y,2)/pow(r.y,2)) + (pow(v.z,2)/pow(r.z,2)));
}

float formeFunction(vec2 v, vec2 r)
{
	vec2 v_abs = abs(v); // Pour AMD : zarb mais sans ça, ça foir l'eval
	return ( (pow(v_abs.x,2)/pow(r.x,2)) + (pow(v_abs.y,2)/pow(r.y,2)) );
}

float matmul_XVX(mat2 V,vec2 X)
{
	// La triple multiplication Xt * V * X ne fonctionne pas de base

	mat2 t_X = mat2(vec2(X.x,0.0), vec2(X.y,0.0));
	vec2 right = V * X;

	return (t_X * right).x;
}

float matmul_XVX(mat3 V,vec3 X)
{
	// La triple multiplication Xt * V * X n'existe pas de base

	mat3 t_X = mat3(vec3(X.x,0.0,0.0), vec3(X.y,0.0,0.0),vec3(X.z,0.0,0.0));
	vec3 right = V * X;

	return (t_X * right).x;
}

float ellipticalGaussian(mat2 V, vec2 X )
{
	mat2 _V = inverse(V);
	return clamp( ( 1.0f / (2.0f * PI * pow(determinant(V), 0.5f ) ) ) * exp( -0.5f * (matmul_XVX(_V,X)) ) , 0.0 , 1.0 );
}

/**
*	Order independent transparency weighting function
*/
float weightFunction(float depth)
{
	//return max(1.0 , 1.0 + exp(-depth/10));
	return clamp(100.0 / max(1.0 + exp(1.0+max(depth,0.01)),0.1),1.0,1000.0);
}

float macGuireEq7(float depth)
{
	return clamp( 10.0 / (0.00005 + pow(abs(depth) / 5.0, 3) + pow(abs(depth) / 200.0, 6) ) , 0.01, 3000.0); 
}

float macGuireEq8(float depth)
{
	return clamp( 10.0 / (0.00005 + pow(abs(depth) / 10.0, 3) + pow(abs(depth) / 200.0, 6) ) , 0.01, 3000.0); 
}

float specialWeightFunction(float Z)
{
	return clamp(1.0 / (0.00005 + pow(abs(Z),6.0)), 1.0, 10000000.0);
}

float macGuireEq9(float depth)
{
	return clamp( 0.03 / (0.00005 + pow(abs(depth) / 200.0, 4) ) , 0.01, 3000.0); 
}

float macGuireEq10(float depth)
{
	return clamp( 10.0 / (0.00005 + pow(abs(depth) / 10.0, 3) + pow(abs(depth) / 200.0, 6) ) , 0.01, 3000.0); 
}

// intersection rayon / AABB
struct Intersection
{
	bool intersect;
	float Ray_distance;
};


// Base algo from RTR 3em edition -> last intersection 
Intersection intersection_RayAABB(vec3 R_o, vec3 R_dir, vec3 AABB_min, vec3 AABB_max)
{
	
	vec3 c = (AABB_min + AABB_max) / 2.0f;
	vec3 h = (AABB_max - AABB_min) / 2.0f;
	vec3 p = c - R_o; 
	
	float swapper;
	
	float tmin = -10000.0f;
	float tmax = 10000.0f;
	
	for(int i = 0; i < 3 ; i++)
	{
		float e = p[i];
		float f = R_dir[i];
		float _f = 1.0f / f;
		if(abs(f) > 0.01)
		{
			float t1 = (e + h[i]) * _f;
			float t2 = (e - h[i]) * _f;
			if(t1 > t2)
			{
				swapper = t2;
				t2 = t1;
				t1 = swapper;
			}
			if(t1 > tmin) tmin = t1;
			if(t2 < tmax) tmax = t2;
			if(tmin > tmax) return Intersection(false,0);
			if(tmax < 0) return Intersection(false,0);
		}
		else
		{
			if( -e - h[i] > 0 || -e + h[i] < 0) return Intersection(false,0);
		}		
	}
	
	//if(tmin > 0) return Intersection(true,tmin);
	return Intersection(true,tmax);	// Pas besoin de la premiere, uniquement de la derniere intersection avec la AABB
}

// Base algo from RTR 3em edition -> last intersection 
Intersection intersection_RayTriangle(vec3 R_o, vec3 R_dir, vec3 P0, vec3 P1, vec3 P2)
{
	vec3 e1 = P1 - P0;
	vec3 e2 = P2 - P0;
	
	vec3 q = cross(R_dir,e2);
	float a = dot(e1,q);
	if(a > - 0.01 && a < 0.01) return Intersection(false,0);
	
	float f = 1.0/a;
	vec3 s = R_o - P0;
	float u = f * dot(s,q);
	if(u < 0) return Intersection(false,0);
	vec3 r = cross(s,e1);
	float v = f * dot(R_dir,r);
	if(v < 0 || (u + v) > 1) return Intersection(false,0);
	
	
	//if(tmin > 0) return Intersection(true,tmin);
	return Intersection(true,f * dot(e2,q));	// Pas besoin de la premiere, uniquement de la derniere intersection avec la AABB
}

vec3 transferToAABB(vec3 point,vec3 AABBmin, vec3 AABBmax)
{
	return (point - AABBmin) / (AABBmax - AABBmin);
}


struct RaySample
{
	vec3 color;
	float alpha;
	float depth;
};

mat4 createKernelSpace(int kernelIndex, vec3 position, vec3 tangent, vec3 biTangent, vec3 normal, vec4 distribution, float scale, float transitionVar )
{
	
	mat3 TBN = mat3(tangent,biTangent,normal);
	// Espace de transformation de l'espace du noyau dans l'espace objet (espace locale élémentaire)
	mat4 kernelToObjectSpace = mat4(1.0);
	kernelToObjectSpace[1].xyz = tangent;
	kernelToObjectSpace[0].xyz = normal;
	kernelToObjectSpace[2].xyz = normalize(cross(kernelToObjectSpace[0].xyz,kernelToObjectSpace[1].xyz));
	kernelToObjectSpace[3].xyz = position;
	kernelToObjectSpace[3].w = 1.0;
	
	
	float theta = randomIn(-PI, PI);
	float phi = randomIn(-PI_4, PI_4);
	//float randomZ = randomIn(-PI, PI);
	float randomZ = 0;
	
	
	float thetaMap = 0.0;
	float phiMap = 0.0;

	bool mode = false;
	if(mode)
	{
		// Rotation spéciale pour schéma : distribution map = rotation random * un facteur d'effet
		thetaMap = randomIn(-PI, PI) * distribution.x;
		phiMap = randomIn(-PI_4, PI_4) * distribution.y;
	}
	else
	{
		vec3 newNormal = distribution.xyz;
		newNormal = (2.0*newNormal ) - 1.0 ; // renormalization
		
		vec3 normalFromTSpace = normalize(TBN * newNormal); // repassage en objet
		vec3 dirFromTSpace = normalize(tangent);
		if(length(newNormal.xy) > 0.0 )
		{
			dirFromTSpace = normalize(TBN * vec3(newNormal.xy,0) );
			//randomZ = 0;
		} // endIf length(newNormal.xy) > 0

		// calcul des angles de rotations dans la carte
		thetaMap = acos(dot(tangent,dirFromTSpace));
		phiMap = acos(dot(normal,normalFromTSpace));

	} // endIf !usingStandardNormalMap

	theta = kernels[kernelIndex].data[4].y * thetaMap + (1.0 - kernels[kernelIndex].data[4].y) * theta;
	phi = kernels[kernelIndex].data[4].y * phiMap + (1.0 - kernels[kernelIndex].data[4].y) * phi;

	if(kernels[kernelIndex].data[2].w > 0.5) theta = 0;

	kernelToObjectSpace[1].xyz = rotateAround(kernelToObjectSpace[1].xyz,theta,kernelToObjectSpace[0].xyz);
	kernelToObjectSpace[0].xyz = rotateAround(kernelToObjectSpace[0].xyz,phi,kernelToObjectSpace[1].xyz);
	kernelToObjectSpace[1].xyz = rotateAround(kernelToObjectSpace[1].xyz,randomZ,kernelToObjectSpace[0].xyz);
	kernelToObjectSpace[2].xyz = normalize(cross(kernelToObjectSpace[0].xyz,kernelToObjectSpace[1].xyz));
	
	// rescaling
	vec3 kernelScale = vec3( transitionVar * kernels[kernelIndex].data[6].xyz + (1.0 - transitionVar) * vec3(1.0)) * f_Grid_height * (kernels[kernelIndex].data[6].w * scale);
	kernelToObjectSpace[0].xyz *= kernelScale.x;
	kernelToObjectSpace[1].xyz *= kernelScale.y;
	kernelToObjectSpace[2].xyz *= kernelScale.z;
	
	return kernelToObjectSpace;
}

float computeKernel(vec3 position , vec3 axisSize, float power  )
{
	mat3 Vrk = mat3(vec3(axisSize.x,0,0),vec3(0,axisSize.y,0),vec3(0,0,axisSize.z));
	mat3 _Vrk = inverse(Vrk);
	float distanceEval = matmul_XVX(_Vrk,position);
	float dVrk = sqrt(determinant(Vrk));
	float g_vrk =  (1.0 / (2.0 * PI * dVrk)) * exp(-0.5 * distanceEval);
	g_vrk = clamp( power * g_vrk,0.0, 1.0);
	return g_vrk;
}


// retest de la reduction de redondance :
int voxelIdAlreadyTested[27];
int voxelIdTested[27];


out vec4 outColor;

void main()
{
	outColor = vec4(0.0);
	
	bool isOnSurface = false;
	
	vec2 v2_Screen_coords =  gl_FragCoord.xy / v2_Screen_size;
	vec2 v2_Screen_coordsUp =    v2_Screen_coords + dFdx(v2_Screen_coords);
	vec2 v2_Screen_coordsRight =  v2_Screen_coords + dFdy(v2_Screen_coords);
		
	mat4 m44_Camera_toObject = inverse(m44_Object_toCamera);
	
	vec3 v3_Object_cameraRight = (m44_Camera_toObject * vec4(1,0,0,0)).xyz;	
	vec3 v3_Object_cameraUp = (m44_Camera_toObject * vec4(0,1,0,0)).xyz;	
	vec3 v3_Object_cameraDir = (m44_Camera_toObject * vec4(0,0,1,0)).xyz;	
	
	vec3 v3_AABB_vector = v3_AABB_max - v3_AABB_min;
	
	// rayon de vue
	vec3 v3_Ray_origin = _vertex.Position.xyz;
	vec3 v3_Ray = v3_Ray_origin - v3_Object_cameraPosition;
	vec3 v3_Ray_direction = normalize(v3_Ray);
	
	
	// Calcul d'intersection entre AABB et le rayon de vue
	Intersection is_ray_AABB = intersection_RayAABB(v3_Ray_origin,v3_Ray_direction, v3_AABB_min, v3_AABB_max);
	vec3 v3_Ray_end = v3_Ray_origin + v3_Ray_direction * is_ray_AABB.Ray_distance;
	
	// depth
	float f_Ray_endDepth = -(m44_Object_toCamera * vec4(v3_Ray_end,1.0)).z;
	float f_MaxDepth = f_Ray_endDepth;
	
	vec4 depthTexture = texture(smp_DepthTexture,v2_Screen_coords);
	if(depthTexture.a == 1.0)
	{
		f_MaxDepth = depthTexture.x;
		isOnSurface = true;
	}
	
	if(f_Ray_endDepth > f_MaxDepth) 
	{
		float scaling = f_MaxDepth / f_Ray_endDepth;
		// retronquer le rayon 
		v3_Ray_end = v3_Ray_origin + v3_Ray_direction * (is_ray_AABB.Ray_distance * scaling) ;
	}
	
	// NOTE : il faudrait aussi arreter le rayon si la surface est sous un objet
	
	// clamping des coordonnées dans un espace [0 ; 1[
	vec3 v3_AABB_rayOriginCoord = (v3_Ray_origin - v3_AABB_min) / v3_AABB_vector;
	vec3 v3_AABB_rayEndCoord = (v3_Ray_end - v3_AABB_min) / v3_AABB_vector;
	v3_AABB_rayOriginCoord = clamp(v3_AABB_rayOriginCoord,vec3(0),vec3(0.9999));
	v3_AABB_rayEndCoord = clamp(v3_AABB_rayEndCoord,vec3(0),vec3(0.9999)) ;
	
	
	// infos du rayon 
	vec3 pos = v3_AABB_rayOriginCoord * iv3_VoxelGrid_subdiv;
	ivec3 currentCell = ivec3(floor(pos)); 
	
	
	
	
	
	
	
	bool useAA = activeAA;
	bool renderShadows = activeShadows;
	
	
	
	
	vec4 bestColor = vec4(0.0);
	float bestDepth = f_MaxDepth;
	float bestAlpha = 0.0;

	vec4 sommeCk = vec4(0.0);
	float sommeAlphaK = 0.0;
	
	bool isReferenceCell = ( currentCell.x == 0 && currentCell.y == 0 && currentCell.z == 0);
	
	bool isEvaluable = false;
	
	vec3 currentRayPoint = v3_AABB_rayOriginCoord;

	
	vec4 testingUV = vec4(0,0,0,1.0);
		
	float endOfPath = initGrid(v3_AABB_rayOriginCoord,v3_AABB_rayEndCoord,iv3_VoxelGrid_subdiv);

	// voxel with redundancy avoidance
	for(int i = 0 ; i < 27; ++i) voxelIdTested[i] = voxelIdAlreadyTested[i] = -2;
	

	
	for(int path = 0; path <= endOfPath &&  sommeAlphaK < 1.0; ++path) 
	{
		
		for(int i = 0; i < 27; ++i) voxelIdAlreadyTested[i] = voxelIdTested[i];		

		// version SSBO
		int index = compute1DGridIndex(currentCell,iv3_VoxelGrid_subdiv);
		vec4 data1 = voxels[index].data[0];
		vec4 data2 = voxels[index].data[1];
		
		// version texture 3D
		//vec4 data1 = texelFetch(smp_voxel_UV,currentCell,0);;
		//vec4 data2 = texelFetch(smp_voxel_distanceField,currentCell,0);;
		
		vec4 sommeCc = vec4(0.0);
		float sommeAlphaC = 0.0;
		
		int z_i = 0;
		int y_i = 0;
		int x_i = 0;
		
		
		int indexOfCell = 0;
		// voxel voisins
		for(z_i = -1 ; z_i <= 1 && data2.x < f_Grid_height ; ++z_i)
		for(y_i = -1 ; y_i <= 1; ++y_i)
		for(x_i = -1 ; x_i <= 1; ++x_i)
		{
			vec4 sommeCj = vec4(0.0);
			float sommeAlphaJ = 0.0;
		
			ivec3 evaluationVoxel = clamp(currentCell+( ivec3(x_i ,  y_i  , z_i  ) ), ivec3(0), ivec3(iv3_VoxelGrid_subdiv - 1));
			// Plan et distance field du voxel
			int VoxelIndex = compute1DGridIndex(evaluationVoxel,iv3_VoxelGrid_subdiv);
			vec4 dataUV = voxels[VoxelIndex].data[0];
			vec4 dataDis = voxels[VoxelIndex].data[1];
			vec2 closestUV = voxels[VoxelIndex].data[2].xy;
			//vec4 dataUV = texelFetch(smp_voxel_UV,evaluationVoxel,0);;
			//vec4 dataDis = texelFetch(smp_voxel_distanceField,evaluationVoxel,0);;
			
			float distanceField = dataDis.x;
			vec2 texAreaMin = dataUV.xy;
			vec2 texAreaMax = dataUV.zw;

			voxelIdTested[indexOfCell] = VoxelIndex;
			bool isNotAlreadyAcc = true;
			for(int i = 0; i < 27 && isNotAlreadyAcc; ++i) 
			{
				if(voxelIdAlreadyTested[i] == VoxelIndex) isNotAlreadyAcc = false;
			}
			indexOfCell++;

			if(isNotAlreadyAcc)
			{
				
				vec2 texAreaVector = texAreaMax - texAreaMin;
				vec2 cellCenter = texAreaMin + texAreaVector * 0.5;
				
				//vec2 texAreaVector = min(abs(texAreaMax - closestUV), abs(texAreaMin - closestUV));
				//vec2 cellCenter = closestUV;

				//initRandom(VoxelIndex);
				vec2 randomInitTest;
				randomInitTest.x = evaluationVoxel.x * evaluationVoxel.z;
				randomInitTest.y = evaluationVoxel.y;
				initRandom(randomInitTest);
				
				// distribution circulaire
				float transition =  pow(min(float(evaluationVoxel.x) / float(iv3_VoxelGrid_subdiv.x),1.0),2.0) ;
				transition = 1.0;
				
				bool activeLoop = false;
				if(distanceField <= f_Grid_height) 
				{		
					activeLoop = true;
					isEvaluable = true;
				}
				
				
				// Du coup les noyaux partage le même seed du PRNG
				for(int i = 0; i < nbModels && activeLoop && renderKernels ; i++)
				{
					bool activeKernel = kernels[i].data[0].x > 0.5;
					// Density per cell devient density per voxel
					float d_max = kernels[i].data[2].x * length(texAreaVector) * 0.5;



					for(int k = 0; k < kernels[i].data[2].z  && activeKernel; k++)
					{
						
						vec2 randomCellPosition = cellCenter + clamp(randomIn(0,d_max) * normalize(randomIn(vec2(-1),vec2(1))), vec2(-1), vec2(1));
						
						// Récupération des données stocker dans les textures
						// Surface de l'objet
						vec4 positionMap = textureLod(smp_surfaceData,vec3(randomCellPosition,0), 0);
						vec3 normalMap = normalize( textureLod(smp_surfaceData,vec3(randomCellPosition,1), 0).xyz );
						vec3 tangentMap = normalize( textureLod(smp_surfaceData,vec3(randomCellPosition,2), 0).xyz );
						
						if(kernels[i].data[2].w > 0.5) tangentMap = normalize(v3_Object_cameraRight);

						vec3 biTangentMap = normalize( textureLod(smp_surfaceData,vec3(randomCellPosition,3), 0).xyz );

						vec4 densityMap = vec4(0);
						vec4 distributionMap = vec4(0,0,1,0);
						vec4 scaleMap = vec4(0);
						
						// Carte de controle
						densityMap = textureLod(smp_densityMaps,vec3(randomCellPosition,int(kernels[i].data[3].x)),0);
						distributionMap = textureLod(smp_distributionMaps,vec3(randomCellPosition,int(kernels[i].data[3].y)), 0);
						scaleMap = textureLod(smp_scaleMaps, vec3(randomCellPosition,int(kernels[i].data[3].z)),0);
						
						// Influence des cartes de controle sur les données aléatoires
						float density =  kernels[i].data[4].x * densityMap.x + (1.0 -  kernels[i].data[4].x) * randomIn(0.0,1.0);
						float scale =  kernels[i].data[4].z * scaleMap.x + (1.0 -  kernels[i].data[4].z) * randomIn(0.0,1.0);
						// manque une extraction plus cohérentes des données issues du 

						int kernelTextureID = int(randomIn(kernels[i].data[0].y, kernels[i].data[0].z + 0.9));
						//kernelTextureID = int(kernels[i].data[0].y);

						// chance d'apparaitre d'un noyau
						float chanceOfPoping = random();
						

						// Si le noyau peut être créer (si sa position existe et qu'il a une chance d'aparaitre)
						if(positionMap.w == 1.0 && density > 0.0 && chanceOfPoping <= density && scale > 0.0)
						{
						
							
							mat4 kernelToObjectSpace = createKernelSpace(i, positionMap.xyz, tangentMap, biTangentMap, normalMap, distributionMap, scale, transition);
							
							if(!modeSlicing)
							{
								// Splatting : projection du noyau dans l'espace écran
								mat3 kernelToScreenSpace = project2DSpaceToScreen(kernelToObjectSpace[3].xyz , kernelToObjectSpace[0].xyz , kernelToObjectSpace[1].xyz, m44_Object_toScreen);
								vec3 originalPosition = projectPointToScreen(kernelToObjectSpace[3].xyz,m44_Object_toScreen );
								
								// pour évaluation du pixel dans l'espace du noyau : projection inverse
								mat3 screenToKernelSpace = inverse(kernelToScreenSpace);
								vec3 kernelFragmentPosition = screenToKernelSpace * vec3(v2_Screen_coords,1.0);
								vec3 kernelFragmentRight = screenToKernelSpace * vec3(v2_Screen_coordsRight,1.0);
								vec3 kernelFragmentUp = screenToKernelSpace * vec3(v2_Screen_coordsUp,1.0);
								kernelFragmentPosition.z = 0.0;
								
								// sample dans l'espace camera pour calcul de la profondeur
								vec3 object_point = (kernelToObjectSpace * vec4(kernelFragmentPosition,1.0)).xyz; 
								vec4 pointInCamera = m44_Object_toCamera * vec4( object_point , 1.0);
								pointInCamera.z = abs(pointInCamera.z);

								vec3 VV = (object_point.xyz - positionMap.xyz);
								float height = dot(normalize(normalMap.xyz),VV);
								
								// Jacobienne du noyau dans l'espace écran
								vec2 dx = kernelFragmentRight.xy - kernelFragmentPosition.xy;
								vec2 dy = kernelFragmentUp.xy - kernelFragmentPosition.xy;
								mat2 J = mat2(dx,dy);
								// Gaussienne dans l'espace du noyau (a paramétrer avec rx et ry)
								mat2 Vrk = mat2(vec2(kernels[i].data[1].x,0),vec2(0,kernels[i].data[1].y));
								// filtrage : application de la jacobienne
								Vrk += (J * transpose(J));
								mat2 _Vrk = inverse(Vrk);
								float d_Vrk = sqrt(determinant(_Vrk));
								float dVrk = sqrt(determinant(Vrk));
								float distanceEval = matmul_XVX(_Vrk,kernelFragmentPosition.xy);
								float g_vrk =  (1.0 / (2.0 * PI * dVrk)) * exp(-0.5 * distanceEval);
								g_vrk = clamp( kernels[i].data[1].w * g_vrk,0.0, 1.0);

								// adaptation du calcul pour choix de resultat
								vec2 kernelTextureCoordinates = kernelFragmentPosition.xy;
								vec2 kernelTextureDx = vec2(0.0);
								vec2 kernelTextureDy = vec2(0.0);
								
								kernelTextureCoordinates.y = kernelFragmentPosition.y * 0.5 + 0.5;
								
								if(useAA)
								{
									kernelTextureDx = kernelFragmentRight.xy;
									kernelTextureDx.y = kernelFragmentRight.y * 0.5 + 0.5;

									kernelTextureDy = kernelFragmentUp.xy;
									kernelTextureDy.y = kernelFragmentUp.y * 0.5 + 0.5;
									
									kernelTextureDx = kernelTextureDx - kernelTextureCoordinates;
									kernelTextureDy = kernelTextureDy - kernelTextureCoordinates;
									
									//kernelTextureDx *= testFactor;
									//kernelTextureDy *= testFactor;
									
								}
								

								vec3 objectPosition = (kernelToObjectSpace * vec4(kernelFragmentPosition,1.0)).xyz;
								vec3 objectNormal = kernelToObjectSpace[2].xyz;

								vec3 worldPosition = ( m44_Object_toWorld * vec4(objectPosition,1.0)).xyz;
								vec3 worldNormal = (m44_Object_toWorld * vec4(objectNormal,0.0)).xyz;
								vec3 worldGroundNormal =  (m44_Object_toWorld * vec4(normalMap.xyz,0.0)).xyz;

								if( kernelFragmentPosition.x > 0  &&  g_vrk > 0.01 && pointInCamera.z < f_MaxDepth && originalPosition.z > 0.0)
								{
									// -- texturing simple :
									vec4 kernelColor = vec4(0.0);
									if( all( bvec2( all(greaterThan(kernelTextureCoordinates,vec2(0.01))) , all(lessThan(kernelTextureCoordinates,vec2(0.99)))) ))
									{
										kernelColor = textureGrad(smp_models,vec3(kernelTextureCoordinates, kernelTextureID ), kernelTextureDx, kernelTextureDy );
									}
									//if( all(greaterThan(kernelTextureCoordinates,vec2(0.01))) )
									//{
									//	kernelColor = g_vrk * vec4(0.05,0.5,0,1);
									//	kernelColor = textureGrad(smp_models,vec3(kernelTextureCoordinates, kernelTextureID ), kernelTextureDx, kernelTextureDy );
									//}
									//outColor = 	g_vrk * vec4(0.05,0.5,0,1);
									

									float VolumetricOcclusionFactor = height / (f_Grid_height * (kernels[i].data[6].w));
									vec4 kernelContrib;

									kernelContrib = addBoulanger2(	
									//kernelContrib = addBoulangerFixe(	
																	worldPosition,
																	kernelColor,
																	worldNormal,
																	worldGroundNormal,
																	//1.0,
																	VolumetricOcclusionFactor,
																	kernels[i].data[5].z,
																	kernels[i].data[5].x,
																	kernels[i].data[5].y,
																	kernels[i].data[5].w
																);
									// test
									//kernelContrib.rg = kernelTextureCoordinates.rg;
									//kernelContrib.b = 0;

									//float newAlpha = 1.0; 
									float newAlpha = weightFunction(pointInCamera.z);
									//if(testFactor >= 1.0)	newAlpha = macGuireEq8(pointInCamera.z);
									//if(testFactor >= 2.0)	newAlpha = macGuireEq7(pointInCamera.z);

									if(kernelContrib.a > 0.01)
									{
										
										//if(pointInCamera.z < bestDepth && testFactor >= 3.0)
										if(pointInCamera.z <= bestDepth)
										{

											bestColor.rgb = (kernelContrib.rgb * kernelContrib.a) + (bestColor.rgb * (1.0 - kernelContrib.a));
											bestAlpha =  kernelContrib.a;
											bestDepth = pointInCamera.z;
										}
										sommeCj.rgb += (kernelContrib.rgb * kernelContrib.a * newAlpha );
										sommeCj.a += kernelContrib.a * newAlpha ;
										sommeAlphaJ += kernelContrib.a;
										
									}

								} // endIf kernelFragmentPosition.x > 0  &&  evalTest <= 1.5 && pointInCamera.z < f_MaxDepth && pointInCamera.z > 0.0
							} // endIf !slicing
							else // Slicing mode for volumetric kernel
							{
								
								mat4 objectToKernelSpace = inverse(kernelToObjectSpace);
								// 1er recalculé le rayon camera -> kernel rescaler par rapport au noyau
								vec3 v3_Object_KernelCameraDir = normalize(positionMap.xyz - v3_Object_cameraPosition);
								v3_Object_KernelCameraDir = (kernelToObjectSpace * vec4(normalize( (objectToKernelSpace * vec4(v3_Object_KernelCameraDir,0.0)).xyz),0)).xyz;
								
								int nbSlice = 5;
								
								float sommeAlphaS = 0.0;
								vec3 sommePos = vec3(0.0);
								vec3 sommeNormal = vec3(0.0);
								
								vec3 originalPosition = projectPointToScreen(kernelToObjectSpace[3].xyz,m44_Object_toScreen );
								
								for(int sl = 1; sl <= nbSlice && sommeAlphaS < 1.0; ++sl)
								{
									vec3 currentSlicePos = positionMap.xyz - ( 1.0 - float(sl) / (float(nbSlice + 1) * 0.5)) * (v3_Object_KernelCameraDir) ;
									// une slice (orthognal a la caméra) correspond à un espace 2D
									mat3 pixelToSliceSpace = project2DSpaceToScreen(currentSlicePos ,v3_Object_cameraRight , v3_Object_cameraUp, m44_Object_toScreen);
									pixelToSliceSpace = inverse(pixelToSliceSpace);
									vec3 pixelPositionInSlice = pixelToSliceSpace * vec3(v2_Screen_coords,1.0);
									
									pixelPositionInSlice = currentSlicePos + pixelPositionInSlice.x * v3_Object_cameraRight + pixelPositionInSlice.y * v3_Object_cameraUp;

									// version barbar : reprojection du pixel dans la slice et evaluation du noyau 3D
									vec3 pixelPositionInKernel = (objectToKernelSpace * vec4(pixelPositionInSlice,1.0)).xyz;

									vec3 kernelRadius = kernels[i].data[1].xyz;
									float g_vrk = computeKernel(pixelPositionInKernel , kernelRadius, kernels[i].data[1].w );
									sommeAlphaS += g_vrk;
									sommePos += pixelPositionInSlice * g_vrk;
									sommeNormal += g_vrk * normalize(( kernelToObjectSpace * vec4(pixelPositionInKernel,0.0)).xyz);

									// test avec 2em noyau :
									//g_vrk = computeKernel(pixelPositionInKernel ,  kernels[i].data[1].xzy, kernels[i].data[1].w );
									//sommeAlphaS += g_vrk;
									//sommePos += pixelPositionInSlice * g_vrk;
									//sommeNormal += g_vrk * normalize(( kernelToObjectSpace * vec4(pixelPositionInKernel,0.0)).xyz);
								}
								if(sommeAlphaS >= 1.0)
								{
									sommePos /= sommeAlphaS;
									sommeNormal /= sommeAlphaS;
								}
								else
								{
									sommeAlphaS = 0;
								}
								
								sommeAlphaS = testFactor * min(sommeAlphaS,1.0);
								
								sommeNormal = normalize(sommeNormal);
								vec3 objectPosition = sommePos;
								
								// recalcul de la normal
								vec3 objectNormal = sommePos - positionMap.xyz;
								//vec3 objectNormal = sommeNormal;
								
								vec3 VV = (objectPosition.xyz - positionMap.xyz);
								float height = dot(normalize(normalMap.xyz),VV);
								
								// 
								vec4 pointInCamera = m44_Object_toCamera * vec4( objectPosition , 1.0);
								pointInCamera.z = -pointInCamera.z;

								vec3 worldPosition = ( m44_Object_toWorld * vec4(objectPosition,1.0)).xyz;
								vec3 worldNormal = (m44_Object_toWorld * vec4(objectNormal,0.0)).xyz;
								vec3 worldGroundNormal =  (m44_Object_toWorld * vec4(normalMap.xyz,0.0)).xyz;

								if( sommeAlphaS > 0.01 && pointInCamera.z < f_MaxDepth)
								{
									vec4 kernelColor = vec4(0.0);
									
									// Changement de couleur linéaire
									//kernelColor = mix( vec4(0.8,0.6,0.1,sommeAlphaS) , vec4(0.6,0.1,0.8,sommeAlphaS) ,transition);
									kernelColor = vec4(0.4,0.8,0.3,sommeAlphaS);
									if(i == 0)	kernelColor = vec4(0.4,0.8,0.3,sommeAlphaS);
									if(i == 1)	kernelColor = vec4(0.2,0.8,1.0,sommeAlphaS);
									//if(i == 2)	kernelColor = vec4(1.0,1.0,0.0,sommeAlphaS);
									
									vec4 kernelContrib;
									kernelContrib = addBoulanger2(	
																	worldPosition,
																	kernelColor,
																	worldNormal,
																	worldGroundNormal,
																	//1.0,
																	height / f_Grid_height,
																	kernels[i].data[5].z,
																	kernels[i].data[5].x,
																	kernels[i].data[5].y,
																	kernels[i].data[5].w
																);

									float newAlpha;
									newAlpha = weightFunction(pointInCamera.z);
									
									
									if(kernelContrib.a > 0.001)
									{
										
										if(pointInCamera.z < bestDepth)
										{

											bestColor.rgb = (kernelContrib.rgb * kernelContrib.a) + (bestColor.rgb * (1.0 - kernelContrib.a));
											bestAlpha =  kernelContrib.a;
											bestDepth = pointInCamera.z;
										}
										
										sommeCj.rgb += (kernelContrib.rgb * kernelContrib.a * newAlpha );
										sommeCj.a += kernelContrib.a * newAlpha ;
										sommeAlphaJ += kernelContrib.a;
										
									}

								} // endIf kernelFragmentPosition.x > 0  &&  evalTest <= 1.5 && pointInCamera.z < f_MaxDepth && pointInCamera.z > 0.0
							}

							
						} // endIf positionMap.w == 1.0 && density > 0.0 && chanceOfPoping <= density && scale > 0.0
					} // endFor(int k = 0; k < kernels[i].distribution.z && isNotAlreadyAcc; k++)
					


				} // End for each models of kernel
				if(sommeCj.a > 0.0)  sommeCj /= sommeCj.a;
				if(sommeAlphaJ > 1.0) sommeAlphaJ = 1.0;
				sommeCc += sommeCj * sommeAlphaJ;
				sommeAlphaC += sommeAlphaJ;
				
			} // end If isNotAlreadyEval
			
		} // end for each neighbouring voxel x,y,z
	
		if(sommeAlphaC > 0.0) sommeCc /= sommeAlphaC;
		if(sommeAlphaC > 1.0) sommeAlphaC = 1.0;
		sommeCk += sommeCc * sommeAlphaC;
		sommeAlphaK += sommeAlphaC; 
	
		// Bresenham 3D
		currentCell = nextCell(currentCell);
		// test double avancé
		++path;
		currentCell = nextCell(currentCell);

	}
	
	if(sommeCk.a > 1.0) sommeCk /= sommeCk.a;
	if(sommeAlphaK > 1.0) sommeAlphaK = 1.0;

	bestColor = mix(sommeCk,bestColor,bestAlpha);
	outColor = vec4(bestColor.rgb,sommeAlphaK);
	
	// pour des test
	//if( (!isEvaluable) && renderGrid) outColor = vec4(1,0,0,0.5);
	//if((!isEvaluable) && renderGrid && isReferenceCell) outColor = vec4(v3_AABB_rayOriginCoord * iv3_VoxelGrid_subdiv,0.5);
	//if(renderGrid) outColor = testingUV;
	//else if(renderGrid) outColor = vec4(1,0,0,1);
	//else outColor = vec4(0);

}
