#version 430
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Random/Random"				// Pseudo Random Number Generator
#include "/Materials/Common/Grid"						// Outil de subdivision de l'espace
#include "/Materials/Common/Lighting/Lighting"			// Modèles d'illumination								
#line 8

in vec3 texCoord;

// constantes
#ifndef PI 
	#define PI 3.14159265359
	#define PI_2 1.57079632679
	#define PI_4 0.78539816339
#endif
#define NBV 1
#define GRIDSIZE 9

// Variables CPU

struct kernel{
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


layout (std430,binding=3) buffer KernelBuffer
{
	ivec4 nbModels;	
	kernel Kernels[];
};


// Vertex stockés dans la liste des triangles envoyés au GPU
struct Vertex
{
	vec4 Position;
	vec4 TextureCoordinates;
};

struct accu
{
	vec4 color; // Couleur pondéré + alpha pondéré
	float alpha; // alpha véritable
};


layout(std140) uniform FUBO 
{
	mat4 objectToScreen; 	// ModelViewProjection matrix
	mat4 objectToWorld;		// Model matrix
	mat4 objectToCamera;	// ModelView
	mat4 worldToCamera;		// view matrix
	float gridHeight;		// Height of the volumetric grid
	vec4 screenInfo;		// XY : width and height, ZW : zNead and zFar
	vec3 cameraPositionInObjectSpace;
	// Debug
	bool renderKernels;	
	bool activeShadows;
	bool activeAA;

	bool activeWind;
	float windFactor;
	float windSpeed;

	bool activeMouse;
	vec2 mousePos;
	float mouseFactor;
	float mouseRadius;

	float timer;
		
};

uniform sampler2D smp_noiseTexture;
uniform sampler2D smp_shellRayFront;		// RG : Texture coordinates of the entry point, A : 1.0 if value exists, else 0.0
uniform sampler2D smp_shellRayBack; 		// RG : Texture coordinates of the exit point, B : distance camera / exit point, A : 1.0 if value exists, else 0.0
uniform sampler2D smp_shellRaySurface;

uniform sampler2D smp_colorFBO;			// Pre rendered scene
uniform sampler2D smp_colorFBODepth;	// Pre rendered depth
uniform sampler2DArray smp_models;	
uniform sampler2DArray smp_densityMaps;
uniform sampler2DArray smp_scaleMaps;
uniform sampler2DArray smp_distributionMaps;
uniform sampler2DArray smp_surfaceData;

// Variables globales
float maxDepth = 0.0;
vec4 sommeColor = vec4(0.0);
float sommeAlpha = 0.0;
float productAlpha = 1.0;
vec4 sommeCellColor = vec4(0.0);

vec2 screenFragment;
vec2 fragmentRight;
vec2 fragmentUp;
ivec2 alreadyEvaluatedCells[GRIDSIZE];
ivec2 evaluatedCells[GRIDSIZE];


//
//	Fonctions nécessaires au splatting
//
//	
//	@brief	projection dans l'espace écran normalisé d'un point
//	@param	point Point 3D à splatter sur l'écran
//	@return position du point à l'écran
//	
vec3 projectPointToScreen(vec3 point,mat4 pointToScreenSpace)
{

	vec4 point_MVP = pointToScreenSpace * vec4(point,1.0);
	vec3 point_NDC = point_MVP.xyz / point_MVP.w;
	// calcul des coordonnées dans un pseudo espace écran ( passage de [-1;1] a [0;1])
	vec3 pointScreen;
	pointScreen = 0.5 + point_NDC * 0.5; 

	return pointScreen;
}


//	
//	@brief	Projection d'une coupe d'un espace 3D dans l'espace écran
//	@param	point Origine du plan de coupe
//	@param  
//	@return position du point à l'écran
//	
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


float matmul_XVX(mat2 V,vec2 X)
{
	// La triple multiplication Xt * V * X ne fonctionne pas de base

	mat2 t_X = mat2(vec2(X.x,0.0), vec2(X.y,0.0));
	vec2 right = V * X;

	return (t_X * right).x;
}

//
//Order independent transparency weighting function
//
float weightFunction(float Z)
{
	//return max(1.0 , 1.0 + exp(-depth/10));
	return clamp(1000.0 / max(1.0 + exp(1.0+max(Z,0.01)),0.1),1.0,3000.0);
}

float macGuireEq7(float Z)
{
	return clamp( 10.0 / (0.00005 + pow(abs(Z) / 5.0, 3.0) + pow(abs(Z) / 200.0, 6.0) ) , 1.0, 10000.0); 
}

float macGuireEq8(float Z)
{
	return clamp( 10.0 / (0.00005 + pow(abs(Z) / 10.0, 3.0) + pow(abs(Z) / 200.0, 6.0) ) , 1.0, 10000.0); 
}

float macGuireEq9(float Z)
{
	return clamp( 0.03 / (0.00005 + pow(abs(Z) / 200.0, 4.0) ) , 1.0, 10000.0); 
}

float specialWeightFunction(float Z)
{
	return clamp(1.0 / (0.00005 + pow(abs(Z),6.0)), 1.0, 10000000.0);
}



// ************************ Ray from ShellRayCompute
vec4 rayIn(vec2 normalizedScreenCoord)
{
	return textureLod(smp_shellRayFront,normalizedScreenCoord.xy,0);
}


bool isOnSurface = false;

vec4 rayOut(vec2 normalizedScreenCoord)
{
	vec4 surfaceFBO = textureLod(smp_shellRaySurface,normalizedScreenCoord.xy,0);
	if(surfaceFBO.w == 1.0)
	{
		isOnSurface = true;
		return surfaceFBO;
	} 
	else return textureLod(smp_shellRayBack,normalizedScreenCoord.xy,0);
}
// **************************************************

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

out vec4 Color;

/* Checking data
void main()
{
	Color = vec4(0.0);
}

 // */
bool debugTest(bool lastState, bool stateChangeTest)
{
	return stateChangeTest ? true : lastState;
}

// Main -- Bresenham 2D
void main()
{
	int nbCellInStack;
	ivec2 cellStack[9];

	//texCoord = gl_FragCoord.xy / screenInfo.xy;
	screenFragment = texCoord.xy;
	fragmentRight = texCoord.xy + dFdx(texCoord.xy);
	fragmentUp = texCoord.xy + dFdy(texCoord.xy);

	// 1ere étape : Initialisation
	vec4 sceneColor = texture(smp_colorFBO, texCoord.xy);
	vec4 sceneDepth = texture(smp_colorFBODepth,texCoord.xy);
	Color = sceneColor;

	// Splatter to screen space
	//initSplat(objectToScreen,texCoord.xy,screenInfo);
	// Evaluated ray in texture space
	vec4 rayEntryPoint = rayIn(texCoord.xy);
	
	vec4 rayExitPoint = rayOut(texCoord.xy);

	// pour calcul de l'ombrage sur la surface
	vec4 rayExitPoint_right = rayOut(fragmentRight);
	vec4 rayExitPoint_up = rayOut(fragmentRight);

	vec4 rayExitPoint_object = textureLod(smp_surfaceData,vec3(rayExitPoint.xy,0), 0);
	vec4 rayExitPoint_object_right = textureLod(smp_surfaceData,vec3(rayExitPoint_right.xy,0), 0);
	vec4 rayExitPoint_object_up = textureLod(smp_surfaceData,vec3(rayExitPoint_up.xy,0), 0);
	
	//vec3 rayExitGradX = rayExitPoint_object_right.xyz - rayExitPoint_object.xyz;
	//vec3 rayExitGradY = rayExitPoint_object_up.xyz - rayExitPoint_object.xyz;
	bool renderShadows = activeShadows;
	bool useAA = activeAA;

	// Calcul de la profondeur maximal d'évaluation
	maxDepth = sceneDepth.a == 1.0 ? min(sceneDepth.x,rayExitPoint.z) : rayExitPoint.z;

	

	rayEntryPoint = rayEntryPoint.a < 1.0 ? rayExitPoint : rayEntryPoint;
	rayEntryPoint.z = 0.0;

	vec4 bestColor = vec4(0.0);
	float bestDepth = maxDepth;
	float bestAlpha = 0.0;

	vec4 sommeCk = vec4(0.0);
	float sommeAlphaK = 0.0;

	float sceneOcclusion = 0.0;

	mat4 modelView = worldToCamera * objectToWorld;
	mat4 viewToModel = inverse(modelView);
	vec3 cameraPosition = (viewToModel * vec4(0.0,0.0,0.0,1.0)).xyz;

	bool debugFlag = false;

	
	// validation des données d'entrées
	bool startRendering = rayExitPoint.a == 1.0 && rayEntryPoint.z < maxDepth && renderKernels;
	if(startRendering)
	{
		//vec3 object_rayDirection = normalize(rayExitPoint_object.xyz - cameraPosition.xyz);
		Color = vec4(0.0);
		// 3ème étape : calcul du pattern
		//for(int i = 0; i < nbModels.x; i++) 
		{	
			// Copie local du kernel pour les calcul
			kernel computeKernel = Kernels[0];
			int nbCellPerLine = int(computeKernel.data[2].y);
			int kernelPerCell = int(computeKernel.data[2].z);

			sceneOcclusion += textureLod(smp_densityMaps,vec3(rayExitPoint.xy,int(computeKernel.data[3].x)),0).x;
			vec4 sommeCc = vec4(0.0);
			float sommeAlphaC = 0.0;

			float endOfPath = initGrid(	rayEntryPoint.xy, 
										rayExitPoint.xy, 
										nbCellPerLine
										);
			
		
			ivec2 pathCell = cellOf(rayEntryPoint.xy);
			ivec2 lastCell = cellOf(rayExitPoint.xy);
			bool isLastCell = false;;
			for(int g = 0; g < GRIDSIZE; g++) 	alreadyEvaluatedCells[g] = evaluatedCells[g] = ivec2(-2);

			vec2 rayPosition = gridPositionOf(rayEntryPoint.xy);

			float path = 0;
			if(computeKernel.data[0].x > 0.5)
			{
				for(path = 0; path <= endOfPath && sommeAlphaC < 1.0 ; path++)
				{
					// Pour chaque cellule de la grille 
					// Sans voisin
					int x_i = 0;
					int y_i = 0;

					vec4 sommeCj = vec4(0.0);
					float sommeAlphaJ = 0.0;

					isLastCell = all(equal(pathCell,lastCell));

					// on reset le buffer de test d'évaluation a partir des cellules précédemment évaluées
					for(int g = 0; g < GRIDSIZE; g++) 	alreadyEvaluatedCells[g] = evaluatedCells[g];
					int indexOfCell = 0;

					ivec2 cellMin = clamp(pathCell - ivec2(NBV),ivec2(0),ivec2(nbCellPerLine-1));
					ivec2 cellMax = clamp(pathCell + ivec2(NBV),ivec2(0),ivec2(nbCellPerLine-1));
					// Double boucle sur x_i et y_i
					for(x_i = cellMin.x ; x_i <= cellMax.x ; x_i++)
					for(y_i = cellMin.y ; y_i <= cellMax.y ; y_i++)
					{
						
						ivec2 currentCell = ivec2(x_i,y_i);

						// Recalul du zNear et zFar				
						//vec3 object_cellCenter =  textureLod(smp_surfaceData,vec3(realPositionOf(currentCell.xy + vec2(0.5)),0), 0).xyz;
						//vec3 object_cellNormal =  textureLod(smp_surfaceData,vec3(realPositionOf(currentCell.xy + vec2(0.5)),1), 0).xyz;
						//object_cellNormal = normalize(object_cellNormal);
						//object_cellCenter += (0.5 * gridHeight * object_cellNormal);
						//vec3 object_cellCorner = textureLod(smp_surfaceData,vec3(realPositionOf(currentCell.xy ),0), 0).xyz;
						//float object_gridRadius = length(object_cellCorner - object_cellCenter);
						//// calcul dans l'espace camera
						//vec3 camera_cellCenter = (objectToCamera * vec4(object_cellCenter,1.0)).xyz;
						//vec3 camera_cellCorner = (objectToCamera * vec4(object_cellCorner,1.0)).xyz;
						//vec3 camera_gridHalfVector = camera_cellCorner - camera_cellCenter;
						//float camera_gridRadius = length(camera_gridHalfVector);
						//float camera_zNear = camera_cellCenter.z ;
						//float camera_zFar = camera_cellCenter.z ;
						//camera_zNear += (camera_gridRadius );
						//camera_zFar -= (camera_gridRadius );
						// FIN Recalul du zNear et zFar	

						// storing the index of cell being evaluated
						evaluatedCells[indexOfCell] = currentCell.xy;
						
						// On vérifie dans le buffer de cellule déjà évaluée si l'indice de cellul est présent
						bool isNotAlreadyAcc = true;
						for(int i = 0; i < 9; i++)
						{
							// Si c'est le cas, la cellule a déjà été évalué
							//isNotAlreadyAcc = all(equal(alreadyEvaluatedCells[i],currentCell)) ? false : isNotAlreadyAcc;
							if(alreadyEvaluatedCells[i].x == currentCell.x 
								&& alreadyEvaluatedCells[i].y == currentCell.y ) isNotAlreadyAcc = false;
						} 	
						indexOfCell+=1;
						//if(!isNotAlreadyAcc) continue;

						// on distribue N noyau dans la cellule (si la cellul n'est pas déjà évaluée) (tempoff)
						initRandom(currentCell.xy);
						for(int k = 0; k < kernelPerCell && isNotAlreadyAcc; k++)
						{	// Pour chaque Noyau
							// on crée un espace d'évaluation dans l'espace objet positionner aléatoirement sur la surface
							//vec4 kernelBaseColor = vec4(normalize(randomVec3()),1.0);

							// Begin : Génération du noyau

							//vec2 randomCellPosition = realPositionOf(currentCell.xy + randomVec2() * 0.99);
							vec2 randomCellPosition = randomInFrame( vec2(currentCell.xy) + 0.5, vec2(0.0), vec2(0.5) );
							randomCellPosition = realPositionOf(randomCellPosition);
							// Récupération des données stocker dans les textures
							// Surface de l'objet
							vec4 positionMap = textureLod(smp_surfaceData,vec3(randomCellPosition,0), 0);
							vec3 normalMap = normalize( textureLod(smp_surfaceData,vec3(randomCellPosition,1), 0).xyz );
							vec3 tangentMap = normalize( textureLod(smp_surfaceData,vec3(randomCellPosition,2), 0).xyz );
							vec3 biTangentMap = normalize( textureLod(smp_surfaceData,vec3(randomCellPosition,3), 0).xyz );


							vec4 densityMap = vec4(0);
							vec4 distributionMap = vec4(0,0,1,0);
							vec4 scaleMap = vec4(0);
							
							// Carte de controle
							densityMap = textureLod(smp_densityMaps,vec3(randomCellPosition,int(computeKernel.data[3].x)),0);
							distributionMap = textureLod(smp_distributionMaps,vec3(randomCellPosition,int(computeKernel.data[3].y)), 0);
							scaleMap = textureLod(smp_scaleMaps, vec3(randomCellPosition,int(computeKernel.data[3].z)),0);


							
							// wind function 
							//float wind =  
							//sin(
							//	sin( 
							//		sin(
							//			sin(randomCellPosition.x + timer * 5.0) + cos(randomCellPosition.y+ timer) + randomCellPosition.y
							//			) * 0.7
							//		) + (timer*2.0)
							//	) + sin(randomCellPosition.y * 10.0+ timer);
							float speed = windSpeed;

							vec2 windPos = randomCellPosition.xy * timer * speed;

							vec4 windTexture = textureLod(smp_noiseTexture,vec2(windPos),0);
							float wind = (sin( sin(windPos.x * 10.0 * sin(timer * speed) + timer * speed * 1.0) 
        					* sin(windPos.y * 0.25 + timer * speed) * 10.0 )) ;
							// Fonctions de GPU gems 3 ch06
							float windSin = (cos( timer * speed + windPos.x * PI) 
											* cos(timer * speed + windPos.x * 3.0 * PI) 
											* cos(timer * speed + windPos.x * 5.0 * PI)
											* cos(timer * speed * 7.0 + windPos.x * 7.0 * PI)
											+ sin(windPos.y * 25.0 * PI)) * 0.25;
							wind = clamp(windSin,0.0,0.5) * windTexture.x;

							vec3 originalSPosition = projectPointToScreen(positionMap.xyz,objectToScreen );
							float mouse = 0.0;
							if (length(mousePos*0.5+0.5 - originalSPosition.xy) < mouseRadius)
							{
								mouse = pow(1.0 - length(mousePos*0.5+0.5 - originalSPosition.xy) / mouseRadius,4.0);

							}
							
							// Direction random selon theta et phi
							float theta = randomIn(-PI, PI);
							float phi = randomIn(0, PI_4);
							vec3 randomDir = normalize(vec3(sin(theta)*cos(phi),sin(theta)*sin(phi),cos(theta)));
							// Repasser random dir en espace tangent
							randomDir = (randomDir + 1.0) * 0.5;

							vec3 wantedDir = distributionMap.xyz * computeKernel.data[4].y + (1.0 - computeKernel.data[4].y) * randomDir;

							wind *= windFactor;
							wantedDir.xyz = activeWind ? wind*vec3(1.2,1.2,0.0) + (1.0 - wind)*wantedDir.xyz : wantedDir.xyz;
							
							mouse *= mouseFactor;
							wantedDir.xyz = activeMouse ? mouse*vec3(1.2,1.2,0.0) + (1.0 - mouse)*wantedDir.xyz : wantedDir.xyz;


							// Influence des cartes de controle sur les données aléatoires
							float density =  computeKernel.data[4].x * densityMap.x + (1.0 -  computeKernel.data[4].x) * randomIn(0.0,1.0);
							float scale =  computeKernel.data[4].z * scaleMap.x + (1.0 -  computeKernel.data[4].z) * randomIn(0.0,1.0);

							// manque une extraction plus cohérentes des données issues du 

							int kernelTextureID = int(randomIn(computeKernel.data[0].y, computeKernel.data[0].z + 0.9));
							
							// chance d'apparaitre d'un noyau
							float chanceOfPoping = random();
							
							// Si le noyau peut être créer (si sa position existe et qu'il a une chance d'aparaitre)
							if(positionMap.w == 1.0 && density > 0.0 && chanceOfPoping <= density && scale > 0.0)
							{
								
								mat3 TBN = mat3(tangentMap,biTangentMap,normalMap);
								
								// Espace de transformation de l'espace du noyau dans l'espace objet (espace locale élémentaire)
								mat4 kernelToObjectSpace = mat4(1.0);
								kernelToObjectSpace = createKernelSpace(
															computeKernel, 
															positionMap.xyz, 
															tangentMap,
															normalMap, 
															wantedDir, 
															scale * gridHeight
															);
								

								// Splatting : projection du noyau dans l'espace écran
								mat3 kernelToScreenSpace = project2DSpaceToScreen(kernelToObjectSpace[3].xyz , kernelToObjectSpace[0].xyz , kernelToObjectSpace[1].xyz, objectToScreen);
								vec3 originalPosition = projectPointToScreen(kernelToObjectSpace[3].xyz,objectToScreen );
								
								// pour évaluation du pixel dans l'espace du noyau : projection inverse
								mat3 screenToKernelSpace = inverse(kernelToScreenSpace);
								vec3 kernelFragmentPosition = screenToKernelSpace * vec3(screenFragment,1.0);
								vec3 kernelFragmentRight = screenToKernelSpace * vec3(fragmentRight,1.0);
								vec3 kernelFragmentUp = screenToKernelSpace * vec3(fragmentUp,1.0);
								kernelFragmentPosition.z = 0.0;
								
								// Le problème vient vraisemblablement du calcul de la profondeur:

								vec4 positionInCameraSpace = objectToCamera * kernelToObjectSpace * vec4(kernelFragmentPosition.xy,0,1);
								positionInCameraSpace.z = -positionInCameraSpace.z;
								float distanceToCamera = positionInCameraSpace.z;

								// sample dans l'espace camera pour calcul de la profondeur
								vec4 object_point = (kernelToObjectSpace * vec4(kernelFragmentPosition,1.0)); 
								
								vec3 VV = (object_point.xyz - positionMap.xyz);
								float height = dot(normalize(normalMap.xyz),VV);
								
								// Jacobienne du noyau dans l'espace écran
								vec2 dx = kernelFragmentRight.xy - kernelFragmentPosition.xy;
								vec2 dy = kernelFragmentUp.xy - kernelFragmentPosition.xy;
								mat2 J = mat2(dx,dy);
								// Gaussienne dans l'espace du noyau (a paramétrer avec rx et ry)
								mat2 Vrk = mat2(vec2(computeKernel.data[1].x,0),vec2(0,computeKernel.data[1].y));
								// filtrage : application de la jacobienne
								Vrk += (J * transpose(J));
								mat2 _Vrk = inverse(Vrk);
								float d_Vrk = sqrt(determinant(_Vrk));
								float dVrk = sqrt(determinant(Vrk));
								float distanceEval = matmul_XVX(_Vrk,kernelFragmentPosition.xy);
								float g_vrk =  (1.0 / (2.0 * PI * dVrk)) * exp(-0.5 * distanceEval);
								g_vrk = clamp( 10 * computeKernel.data[1].w * g_vrk,0.0, 1.0);

								

								vec2 kernelTextureCoordinates = ((kernelFragmentPosition.xy) + vec2(0.5,0)).yx;
								vec2 kernelTextureDx = vec2(0.0);
								vec2 kernelTextureDy = vec2(0.0);
								
								kernelTextureDx = ((kernelFragmentRight.xy) + vec2(0.5,0)).yx;
								kernelTextureDy = ((kernelFragmentUp.xy) + vec2(0.5,0)).yx;
								kernelTextureDx = kernelTextureDx - kernelTextureCoordinates;
								kernelTextureDy = kernelTextureDy - kernelTextureCoordinates;
									
								

								vec3 objectPosition = (kernelToObjectSpace * vec4(kernelFragmentPosition,1.0)).xyz;
								vec3 objectNormal = kernelToObjectSpace[2].xyz;

								vec3 worldPosition = ( objectToWorld * vec4(objectPosition,1.0)).xyz;
								vec3 worldNormal = (objectToWorld * vec4(objectNormal,0.0)).xyz;
								vec3 worldGroundNormal =  (objectToWorld * vec4(normalMap.xyz,0.0)).xyz;

								//bool isAValidSample = kernelFragmentPosition.y > 0  && originalPosition.z > 0.0 && pointInCamera.z < maxDepth && g_vrk > 0.01; 
								bool isAValidSample = kernelFragmentPosition.y > 0  
								&&  g_vrk > 0.01 
								&& kernelTextureCoordinates.x < 1.0 
								&& kernelTextureCoordinates.y < 1.0 
								&& kernelTextureCoordinates.x > 0.0 
								&& kernelTextureCoordinates.y > 0.0 
								;

								// Test
								if( !isAValidSample ) continue;
								else
								{
									// -- texturing simple :
									//bool isInTextureSpace = all( bvec2( all(greaterThan(kernelTextureCoordinates,vec2(0.01))) , all(lessThan(kernelTextureCoordinates,vec2(0.99)))) );
									vec4 kernelColor = 	textureGrad(smp_models,vec3(kernelTextureCoordinates, kernelTextureID ), kernelTextureDx, kernelTextureDy ) ;
									//kernelColor = g_vrk * vec4(0.05,0.5,0,1);
									//vec4 kernelColor = isInTextureSpace ? 
									//	textureGrad(smp_models,vec3(kernelTextureCoordinates, kernelTextureID ), kernelTextureDx, kernelTextureDy ) : 
									//	vec4(0.0);
									
										
									vec4 kernelContrib;
									float VolumetricOcclusionFactor = height / (gridHeight * (computeKernel.data[6].w));
									float bendingTest = abs(dot(normalize(worldGroundNormal),normalize(worldNormal)));
									
									//if(bendingTest > 0.5) VolumetricOcclusionFactor = 0.5;
									

									//kernelContrib = vec4(kernelBaseColor.rgb,g_vrk);
									kernelContrib = addBoulanger3(	
																	worldPosition,
	                                   								kernelColor,
	                                   								worldNormal,
	                                   								worldGroundNormal,
	                                   								//1.0,
	                                   								VolumetricOcclusionFactor,
																	computeKernel.data[5].z,
																	computeKernel.data[5].x,
																	computeKernel.data[5].y,
																	computeKernel.data[5].w
	                                   							);
									
									//kernelContrib = vec4(kernelTextureCoordinates.xy,0,g_vrk);

									//float newAlpha = kernelContrib.a;// * weightFunction(distanceToCamera);
									float oitWeight = weightFunction(positionInCameraSpace.z);

									oitWeight *= (1.0 + pow( (height / gridHeight)*100,4.0 ) );

									float newAlpha = kernelContrib.a * oitWeight;

									//float depthInCell =  abs((-pointInCamera.z - camera_zNear) / (camera_zFar - camera_zNear)) ;
									
									//if(testFactor >= 1.5)	newAlpha = macGuireEq7(pointInCamera.z);
									//
									//if(testFactor >= 2.5) 	newAlpha = specialWeightFunction( depthInCell );
									
									// Opti a la con
									//bool isContributive = kernelContrib.a > 0.1;
									//bool isCloser = pointInCamera.z < bestDepth;

									// Mise a jour de la somme
									//sommeCj.rgb = isContributive ? 
									//	sommeCj.rgb + (kernelContrib.rgb * newAlpha ) : 
									//	sommeCj.rgb;
									//sommeCj.a = isContributive ? 
									//	sommeCj.a + newAlpha : 
									//	sommeCj.a;
									//sommeAlphaJ = isContributive ? 
									//	sommeAlphaJ + kernelContrib.a : 
									//	sommeAlphaJ;
																		
									// mise a jour du best sample
									//bestColor.rgb = isCloser && isContributive ? 
									//	(kernelContrib.rgb * kernelContrib.a) + (bestColor.rgb * (1.0 - kernelContrib.a)) : 
									//	bestColor.rgb;
									//bestAlpha = isCloser && isContributive ? 
									//	kernelContrib.a : 
									//	bestAlpha;
									//bestDepth = isCloser && isContributive ? 
									//	pointInCamera.z : 
									//	bestDepth;

									//if(positionInCameraSpace.z > 0)
									if(kernelContrib.a < 0.01) continue;
									else
									{
										//if(testFactor >= 1.5) kernelContrib.rgb = vec3(abs(depthInCell));
										if(distanceToCamera < bestDepth)
										{
											bestColor.rgb = (kernelContrib.rgb * kernelContrib.a) + (bestColor.rgb * (1.0 - kernelContrib.a));
											//bestColor.rgb = vec3(distanceToCamera / maxDepth);
											bestColor.a =  kernelContrib.a;
											bestDepth = distanceToCamera;
										}
										sommeCj.rgb += (kernelContrib.rgb * newAlpha );
										sommeCj.a +=  newAlpha ;
										sommeAlphaJ += kernelContrib.a;
									}

								} // endIf kernelFragmentPosition.x > 0  &&  evalTest <= 1.5 && pointInCamera.z < maxDepth && pointInCamera.z > 0.0


							} // endIf positionMap.w == 1.0 && density > 0.0 && chanceOfPoping <= density && scale > 0.0
						} // endFor(int k = 0; k < computeKernel.distribution.y && isNotAlreadyAcc; k++)

					

						
					}// endFor (y_i = cellMin.y ; y_i <= cellMax.y ; y_i++)
					// endFor (x_i = cellMin.x ; x_i <= cellMax.x ; x_i++)
					
					sommeCj = sommeCj.a > 1.0 ? sommeCj / sommeCj.a : sommeCj;
					sommeAlphaJ = sommeAlphaJ > 1.0 ? 1.0 : sommeAlphaJ;
					
					sommeCc += sommeCj * sommeAlphaJ;
					sommeAlphaC += sommeAlphaJ;

					// Bresenham line next cell
					pathCell = nextCell(pathCell);

					++path;
					pathCell = nextCell(pathCell);
					

				
				}//endFor(float path = 0; path <= endOfPath && computeKernel.distribution.w > 0.5 ; path++)	
				sommeCc = sommeAlphaC > 1.0 ? sommeCc / sommeAlphaC : sommeCc;
				sommeAlphaC = sommeAlphaC > 1.0 ? 1.0 : sommeAlphaC;
				sommeCk += sommeCc * sommeAlphaC;
				sommeAlphaK += sommeAlphaC; 
			} // endif kernel active
			
		} // endFor(int i = 0; i < nbModels && renderKernels; ++i) // fin de l'évaluation de tout les modèles
		sommeCk = sommeCk.a > 1.0 ? sommeCk / sommeCk.a : sommeCk; 
		sommeAlphaK = sommeAlphaK > 1.0 ? 1.0 : sommeAlphaK;
		//bestColor = vec4(bestColor.rgb, bestAlpha);
		
		bestColor.rgb = mix(sommeCk.rgb,bestColor.rgb,bestColor.a);
		
		Color = vec4(bestColor.rgb,sommeAlphaK);
		
	}
	//Color = Kernels[0].data[1];
	//sceneOcclusion = 0.5;

	//if(isOnSurface && renderShadows) sceneColor.rgb = sceneColor.rgb * (1.0 - clamp(sceneOcclusion,0.0,0.25));
	Color = mix(sceneColor,Color,Color.w);

	//Color = rayIn(texCoord.xy);

}

