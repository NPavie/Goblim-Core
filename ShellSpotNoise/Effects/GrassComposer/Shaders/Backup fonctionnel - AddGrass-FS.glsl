
#version 430
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Random/Random"				// Pseudo Random Number Generator
#include "/Materials/Common/Grid"						// Outil de subdivision de l'espace
#include "/Materials/Common/Lighting/Lighting"			// Modèles d'illumination								
#line 8

//in vec3 texCoord;

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
	vec4 data[6];
	// Index of data :
	// 0 -- ModelData :  isActive, model_texIDs_begin, model_texIDs_end
	// 1 -- kernelShape : rx, ry, scale, curvature (pour plus tard)
	// 2 -- distributionData : distribution_ID(pour distribution différentes par noyau), subdivision, densityPerCell, 
	// 3 -- ControlMapsIds : density_texID, distribution_texID, scale_texID, colorMap_texId
	// 4 -- ControlMapsInfluence : densityMap_influence, distributionMap_influence, scaleMap_influence, colorMap_influence
	// 5 -- lightData : attenuation, emittance, reflectance, volumetric obscurance factor
};

layout (std140) uniform KernelUBO
{
	int nbModels;	
	kernel Kernels[10];
} kernelArray;

layout(std140) uniform FUBO 
{
	mat4 objectToScreen; 	// ModelViewProjection matrix
	mat4 objectToWorld;		// Model matrix
	mat4 objectToCamera;	// ModelView
	mat4 worldToCamera;		// view matrix
	float gridHeight;		// Height of the volumetric grid
	vec4 screenInfo;		// XY : width and height, ZW : zNead and zFar
	
	// Debug
	bool renderKernels;	
	bool renderGrid;
	float testFactor;
	float timer;
	// vec4 wind			// maybe later		
};

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


vec2 texCoord;
vec2 screenFragment;
vec2 fragmentRight;
vec2 fragmentUp;
ivec2 alreadyEvaluatedCells[GRIDSIZE];
ivec2 evaluatedCells[GRIDSIZE];


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
	return clamp(500.0 / max(1.0 + exp(1.0+max(depth,0.01)),0.1),1.0,50.0);
}

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


// Main
out vec4 Color;
void main()
{
	int nbCellInStack;
	ivec2 cellStack[9];

	texCoord = gl_FragCoord.xy / screenInfo.xy;
	screenFragment = texCoord.xy;
	fragmentRight = texCoord.xy + dFdx(texCoord.xy);
	fragmentUp = texCoord.xy + dFdy(texCoord.xy);

	// 1ere étape : Initialisation
	vec4 sceneColor = texture(smp_colorFBO, texCoord.xy);
	vec4 sceneDepth = texture(smp_colorFBODepth,texCoord.xy);

	// Splatter to screen space
	//initSplat(objectToScreen,texCoord.xy,screenInfo);
	// Evaluated ray in texture space
	vec4 rayEntryPoint = rayIn(texCoord.xy);
	vec4 rayExitPoint = rayOut(texCoord.xy);
	
	// Calcul de la profondeur maximal d'évaluation
	maxDepth = rayExitPoint.z;
	if(sceneDepth.a == 1.0 ) maxDepth = min(sceneDepth.x, maxDepth);
	

	Color = sceneColor;
	if(rayEntryPoint.a < 1.0) rayEntryPoint = rayExitPoint;
	rayEntryPoint.z = 0.0;

	vec4 bestColor = vec4(0.0);
	float bestDepth = maxDepth;
	float bestAlpha = 0.0;

	vec4 sommeCk = vec4(0.0);
	float sommeAlphaK = 0.0;

	float sceneOcclusion = 0.0;

	// validation des données d'entrées
	if(rayExitPoint.a == 1.0 && rayEntryPoint.z < maxDepth)
	{
		vec4 rayExitPoint_object = textureLod(smp_surfaceData,vec3(rayExitPoint.xy,0), 0);

		Color = vec4(0.0);
		// 3ème étape : calcul du pattern
		for(int i = 0; i < kernelArray.nbModels && renderKernels; i++) 
		//for(int i = kernelArray.nbModels - 1; i >= 0 && renderKernels; i--) 
		{	

			vec4 sommeCc = vec4(0.0);
			float sommeAlphaC = 0.0;

			float endOfPath = initGrid(	rayEntryPoint.xy, 
										rayExitPoint.xy, 
										int(kernelArray.Kernels[i].data[2].y)
										//int(subdiv)
										);
			
		
			ivec2 pathCell = cellOf(rayEntryPoint.xy);
			ivec2 lastCell = cellOf(rayExitPoint.xy);
			bool isLastCell = false;;
			for(int i = 0; i < GRIDSIZE; i++) 	alreadyEvaluatedCells[i] = evaluatedCells[i] = ivec2(-2);
			

			for(float path = 0; path <= endOfPath && kernelArray.Kernels[i].data[0].x > 0.5 && sommeAlphaC < 1.0; path++)
			{	// Pour chaque cellule de la grille 
				// Sans voisin
				int x_i = 0;
				int y_i = 0;

				vec4 sommeCj = vec4(0.0);
				float sommeAlphaJ = 0.0;

				if(pathCell.x == lastCell.x && pathCell.y == lastCell.y) isLastCell = true;

				// on reset le buffer de test d'évaluation a partir des cellules précédemment évaluées
				for(int i = 0; i < GRIDSIZE; i++) 	alreadyEvaluatedCells[i] = evaluatedCells[i];
				int indexOfCell = 0;
				
				for(x_i = -NBV ; x_i <= NBV ; x_i++)
				{
					for(y_i = -NBV ; y_i <= NBV ; y_i++)
					{
						
						ivec2 currentCell = pathCell+ivec2(x_i,y_i);
						// storing the index of cell being evaluated
						evaluatedCells[indexOfCell] = currentCell;
						
						// On vérifie dans le buffer de cellule déjà évaluée si l'indice de cellul est présent
						bool isNotAlreadyAcc = true;
						for(int i = 0; i < 9 && isNotAlreadyAcc; i++)
						{
							// Si c'est le cas, la cellule a déjà été évalué
							if(alreadyEvaluatedCells[i].x == currentCell.x 
								&& alreadyEvaluatedCells[i].y == currentCell.y ) isNotAlreadyAcc = false;
						} 	
						indexOfCell+=1;

						
						// on distribue N noyau dans la cellule (si la cellul n'est pas déjà évaluée)
						initRandom(currentCell);
						for(int k = 0; k < kernelArray.Kernels[i].data[2].z && isNotAlreadyAcc; k++)
						{	// Pour chaque Noyau
							// on crée un espace d'évaluation dans l'espace objet positionner aléatoirement sur la surface
							vec2 randomCellPosition = realPositionOf(currentCell + randomVec2());
							
							// Récupération des données stocker dans les textures
							// Surface de l'objet
							vec4 positionMap = textureLod(smp_surfaceData,vec3(randomCellPosition,0), 0);
							vec3 normalMap = normalize( textureLod(smp_surfaceData,vec3(randomCellPosition,1), 0).xyz );
							vec3 tangentMap = normalize( textureLod(smp_surfaceData,vec3(randomCellPosition,2), 0).xyz );
							vec3 biTangentMap = normalize( textureLod(smp_surfaceData,vec3(randomCellPosition,3), 0).xyz );


							// Carte de controle
							vec4 densityMap = textureLod(smp_densityMaps,vec3(randomCellPosition,int(kernelArray.Kernels[i].data[3].x)),0);
							vec4 distributionMap = textureLod(smp_distributionMaps,vec3(randomCellPosition,int(kernelArray.Kernels[i].data[3].y)), 0);
							vec4 scaleMap = textureLod(smp_scaleMaps, vec3(randomCellPosition,int(kernelArray.Kernels[i].data[3].z)),0);

							// Influence des cartes de controle sur les données aléatoires
							float density =  kernelArray.Kernels[i].data[4].x * densityMap.x + (1.0 -  kernelArray.Kernels[i].data[4].x) * randomIn(0.0,1.0);
							float scale =  kernelArray.Kernels[i].data[4].z * scaleMap.x + (1.0 -  kernelArray.Kernels[i].data[4].z) * randomIn(0.0,1.0);
							// manque une extraction plus cohérentes des données issues du 

							int kernelTextureID = int(randomIn(kernelArray.Kernels[i].data[0].y, kernelArray.Kernels[i].data[0].z + 0.9));
							//kernelTextureID = int(kernelArray.Kernels[i].data[0].y);

							// chance d'apparaitre d'un noyau
							float chanceOfPoping = random();
							


							// Si le noyau peut être créer (si sa position existe et qu'il a une chance d'aparaitre)
							if(positionMap.w == 1.0 && density > 0.0 && chanceOfPoping <= density && scale > 0.0)
							{
								

								mat3 TBN = mat3(tangentMap,biTangentMap,normalMap);
								// Espace de transformation de l'espace du noyau dans l'espace objet (espace locale élémentaire)
								mat4 kernelToObjectSpace = mat4(1.0);
								kernelToObjectSpace[1].xyz = tangentMap;
								kernelToObjectSpace[0].xyz = normalMap;
								kernelToObjectSpace[2].xyz = normalize(cross(kernelToObjectSpace[0].xyz,kernelToObjectSpace[1].xyz));
								kernelToObjectSpace[3].xyz = positionMap.xyz;
								kernelToObjectSpace[3].w = 1.0;
								
								bool usingStandardNormalMap = true;

								float theta = randomIn(-PI, PI);
								float phi = randomIn(-PI_4, PI_4);
								float randomZ = randomIn(-PI, PI);

								float thetaMap = 0.0;
								float phiMap = 0.0;

								if(!usingStandardNormalMap)
								{
									// rotations inscrite dans la carte de distribution
									thetaMap = -PI + distributionMap.x * PI * 2.0;	// direction
									phiMap = -PI_4 + distributionMap.y * PI_2;		// bending
									
									
								}
								else
								{
									vec3 newNormal = distributionMap.xyz;
									newNormal = (2.0*newNormal ) - 1.0 ; // renormalization
									
									vec3 normalFromTSpace = normalize(TBN * newNormal); // repassage en objet
									vec3 dirFromTSpace = normalize(tangentMap);
									if(length(newNormal.xy) > 0.0 )
									{
										dirFromTSpace = normalize(TBN * vec3(newNormal.xy,0) );
										//randomZ = 0;
									} // endIf length(newNormal.xy) > 0

									// calcul des angles de rotations dans la carte
									thetaMap = acos(dot(tangentMap,dirFromTSpace));
									phiMap = acos(dot(normalMap,normalFromTSpace));

								} // endIf !usingStandardNormalMap

								theta = kernelArray.Kernels[i].data[4].y * thetaMap + (1.0 - kernelArray.Kernels[i].data[4].y) * theta;
								phi = kernelArray.Kernels[i].data[4].y * phiMap + (1.0 - kernelArray.Kernels[i].data[4].y) * phi;


								kernelToObjectSpace[1].xyz = rotateAround(kernelToObjectSpace[1].xyz,theta,kernelToObjectSpace[0].xyz);
								kernelToObjectSpace[0].xyz = rotateAround(kernelToObjectSpace[0].xyz,phi,kernelToObjectSpace[1].xyz);
								kernelToObjectSpace[1].xyz = rotateAround(kernelToObjectSpace[1].xyz,randomZ,kernelToObjectSpace[0].xyz);
								kernelToObjectSpace[2].xyz = normalize(cross(kernelToObjectSpace[0].xyz,kernelToObjectSpace[1].xyz));
								
								
								
								
								// rescaling
								vec3 kernelScale = vec3(kernelArray.Kernels[i].data[1].xy,1.0) * gridHeight * (kernelArray.Kernels[i].data[1].z * scale);
								kernelToObjectSpace[0].xyz *= kernelScale.x;
								kernelToObjectSpace[1].xyz *= kernelScale.y;
								kernelToObjectSpace[2].xyz *= kernelScale.z;
								

								// Splatting : projection du noyau dans l'espace écran
								mat3 kernelToScreenSpace = project2DSpaceToScreen(kernelToObjectSpace[3].xyz , kernelToObjectSpace[0].xyz , kernelToObjectSpace[1].xyz, objectToScreen);
								vec3 originalPosition = projectPointToScreen(kernelToObjectSpace[3].xyz,objectToScreen );
								
								
								// problème ici je pense :
								// besoin d'invalider l'espace projeter s'il n'est pas dans l'écran (derriere la camera)
								bool spaceIsValide = true;
								if(kernelToScreenSpace[2].z < 0) spaceIsValide = false;


								// pour évaluation du pixel dans l'espace du noyau : projection inverse
								mat3 screenToKernelSpace = inverse(kernelToScreenSpace);
								vec3 kernelFragmentPosition = screenToKernelSpace * vec3(screenFragment,1.0);
								vec3 kernelFragmentRight = screenToKernelSpace * vec3(fragmentRight,1.0);
								vec3 kernelFragmentUp = screenToKernelSpace * vec3(fragmentUp,1.0);
								kernelFragmentPosition.z = 0.0;
								
								// sample dans l'espace camera pour calcul de la profondeur
								vec3 object_point = (kernelToObjectSpace * vec4(kernelFragmentPosition,1.0)).xyz; 
								vec4 pointInCamera = objectToCamera * vec4( object_point , 1.0);
								pointInCamera.z = -pointInCamera.z;

								vec3 VV = (object_point.xyz - positionMap.xyz);
								float height = dot(normalize(normalMap.xyz),VV);
								
								// Jacobienne du noyau dans l'espace écran
								vec2 dx = kernelFragmentRight.xy - kernelFragmentPosition.xy;
								vec2 dy = kernelFragmentUp.xy - kernelFragmentPosition.xy;
								mat2 J = mat2(dx,dy);
								// Gaussienne dans l'espace du noyau (a paramétrer avec rx et ry)
								mat2 Vrk = mat2(1.0);
								// filtrage : application de la jacobienne
								Vrk += (J * transpose(J));
								mat2 _Vrk = inverse(Vrk);
								float d_Vrk = sqrt(determinant(_Vrk));
								float dVrk = sqrt(determinant(Vrk));
								float distanceEval = matmul_XVX(_Vrk,kernelFragmentPosition.xy);
								float g_vrk =  (1.0 / (2.0 * PI * dVrk)) * exp(-0.5 * distanceEval);
								g_vrk = clamp(g_vrk,0.0, 1.0);

								vec2 kernelTextureCoordinates = kernelFragmentPosition.xy;
								kernelTextureCoordinates.y = kernelFragmentPosition.y * 0.5 + 0.5;

								vec2 kernelTextureDx = kernelFragmentRight.xy;
								kernelTextureDx.y = kernelFragmentRight.y * 0.5 + 0.5;

								vec2 kernelTextureDy = kernelFragmentUp.xy;
								kernelTextureDy.y = kernelFragmentUp.y * 0.5 + 0.5;
								
								kernelTextureDx = kernelTextureDx - kernelTextureCoordinates;
								kernelTextureDy = kernelTextureDy - kernelTextureCoordinates;

								vec3 worldPosition = ( objectToWorld * kernelToObjectSpace * vec4(kernelFragmentPosition,1.0)).xyz;
								vec3 worldNormal = (objectToWorld * vec4(kernelToObjectSpace[2].xyz,0.0)).xyz;
								vec3 worldGroundNormal =  (objectToWorld * vec4(normalMap.xyz,0.0)).xyz;

								if( kernelFragmentPosition.x > 0  &&  g_vrk > 0.01 && pointInCamera.z < maxDepth && originalPosition.z > 0.0)
								{
									// -- texturing simple : 
									vec4 kernelColor = vec4(0.0);
									if( all( bvec2( all(greaterThan(kernelTextureCoordinates,vec2(0.01))) , all(lessThan(kernelTextureCoordinates,vec2(0.99)))) ))
										kernelColor = textureGrad(smp_models,vec3(kernelTextureCoordinates, kernelTextureID ), kernelTextureDx, kernelTextureDy );
										
									vec4 kernelContrib;

									kernelContrib = addBoulanger2(	
																	worldPosition,
	                                   								kernelColor,
	                                   								worldNormal,
	                                   								worldGroundNormal,
	                                   								//1.0,
	                                   								clamp(height,0.0,1.0),
																	kernelArray.Kernels[i].data[5].z,
																	kernelArray.Kernels[i].data[5].x,
																	kernelArray.Kernels[i].data[5].y,
																	kernelArray.Kernels[i].data[5].w
	                                   							);

									float newAlpha = weightFunction(pointInCamera.z);
									
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

								} // endIf kernelFragmentPosition.x > 0  &&  evalTest <= 1.5 && pointInCamera.z < maxDepth && pointInCamera.z > 0.0

								// Calcul de l'occlusion réel du sol à faire ici --------------
								if(isOnSurface && testFactor > 0.01)
								{
									vec3 V0 = worldPosition;
									vec3 N = normalize(worldNormal);
									vec3 L0 = (objectToWorld * vec4(rayExitPoint_object.xyz,1.0)).xyz;
									vec3 W = L0 - V0; 
									for (int i = 0;i < nbLights;i++)
    								{
    									
    									vec3 L = Lights[i].pos.xyz - L0 ;
										float d = length(L);
										L = normalize(L);

										float NL = dot(N,L);
										if(NL > 0.01 || NL < -0.01)
										{
											float NW = dot(-N, W);
											float Si = NW / NL;
											if(Si >= 0)
											{
												vec3 intersectionPoint = L0 + Si * L; // In world space
												intersectionPoint = ( inverse(kernelToObjectSpace) * inverse(objectToWorld) * vec4(intersectionPoint,1.0)).xyz; // to the kernel slice
												float distanceEval = matmul_XVX(Vrk,intersectionPoint.xy);
												if(intersectionPoint.x > 0.0)
												{
													vec2 SiTextureCoordinates = intersectionPoint.xy;
													SiTextureCoordinates.y = intersectionPoint.y * 0.5 + 0.5;

													vec4 kernelColor = vec4(0.0);
													if( all( bvec2( all(greaterThan(SiTextureCoordinates,vec2(0.01))) , all(lessThan(SiTextureCoordinates,vec2(0.99)))) ))
														kernelColor = texture(smp_models,vec3(SiTextureCoordinates, kernelTextureID ));
														//kernelColor = textureGrad(smp_models,vec3(SiTextureCoordinates, kernelTextureID ), kernelTextureDx, kernelTextureDy );
													
													sceneOcclusion += min( testFactor * kernelColor.a, 0.5);
													//sceneOcclusion += 0.2;
												}
											}
										}
										// Sinon perpendicularité
									}
									// intersection rayon ray lumineux / sol
									// si intersection alors reduction de la couleur du sol de (occlusionFactor * alpha de la texture au point d'intersection)
									
								}


							} // endIf positionMap.w == 1.0 && density > 0.0 && chanceOfPoping <= density && scale > 0.0
						} // endFor(int k = 0; k < kernelArray.Kernels[i].distribution.y && isNotAlreadyAcc; k++)
						
					}// endFor (y_i = -NBV ; y_i <= NBV ; y_i++)
				}// endFor (x_i = -NBV ; x_i <= NBV ; x_i++)
				
				if(sommeCj.a > 0.0)  sommeCj /= sommeCj.a;
				if(sommeAlphaJ > 1.0) sommeAlphaJ = 1.0;
				sommeCc += sommeCj * sommeAlphaJ;
				sommeAlphaC += sommeAlphaJ;

				
				//productAlpha *= (1.0 - realAlphaAccumulator);
				pathCell = nextCell(pathCell);

			}	//endFor(float path = 0; path <= endOfPath && kernelArray.Kernels[i].distribution.w > 0.5 ; path++)	
			if(sommeAlphaC > 0.0) sommeCc /= sommeAlphaC;
			if(sommeAlphaC > 1.0) sommeAlphaC = 1.0;
			sommeCk += sommeCc * sommeAlphaC;
			sommeAlphaK += sommeAlphaC; 
		} // endFor(int i = 0; i < kernelArray.nbModels && renderKernels; ++i) // fin de l'évaluation de tout les modèles
		if(sommeCk.a > 1.0) sommeCk /= sommeCk.a;
		if(sommeAlphaK > 1.0) sommeAlphaK = 1.0;

		bestColor = mix(sommeCk,bestColor,bestAlpha);

		// best sample
		if(bestDepth < maxDepth) Color = vec4(bestColor.rgb,sommeAlphaK);
		else Color = vec4(sommeCk.xyz,sommeAlphaK);

		if(renderGrid)
		{
			vec2 testBorderEnd = cellFractOf(rayExitPoint.xy);
			vec2 testBorderBegin = cellFractOf(rayEntryPoint.xy);
			if(testBorderEnd.x < 0.05 || testBorderEnd.y < 0.05 || testBorderEnd.x > 0.95 || testBorderEnd.y > 0.95) Color = vec4(0.0,0.0,1.0,0.5);
			else if(testBorderBegin.x < 0.05 || testBorderBegin.y < 0.05 || testBorderBegin.x > 0.95 || testBorderBegin.y > 0.95) Color = vec4(1.0,0.0,0.0,0.5);	
			else Color = vec4(normalize( textureLod(smp_surfaceData,vec3(rayExitPoint.xy,1), 0).xyz ),1.0);


		}
	}

	//sceneOcclusion = 0.5;

	if(isOnSurface) sceneColor.rgb = sceneColor.rgb * (1.0 - clamp(sceneOcclusion,0.0,0.8));
	Color = mix(sceneColor,Color,Color.w);


}

// */


