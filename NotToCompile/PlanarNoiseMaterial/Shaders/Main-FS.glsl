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





// --------- Paramètres des noyaux d'évaluation  
struct Kernel{
	vec4 data[7];
	// Index of data :
	// 0 -- ModelData :  isActive, model_texIDs_begin, model_texIDs_end
	// 1 -- kernelShape : rx, ry, scale, kernelPower
	// 2 -- distributionData : distribution_ID, subdivision, densityPerCell, 
	// 3 -- ControlMapsIds : density_texID, distribution_texID, scale_texID, colorMap_texId
	// 4 -- ControlMapsInfluence : densityMap_influence, distributionMap_influence, scaleMap_influence, colorMap_influence
	// 5 -- lightData : attenuation, emittance, reflectance, volumetric obscurance factor
	// 6 -- ModelShape : rx, ry, rz
};

layout (std140) buffer KernelBuffer
{
	int nbModels;	
	Kernel kernels[];
}; 

// Et les textures associés
uniform sampler2DArray smp_models;	
uniform sampler2DArray smp_densityMaps;
uniform sampler2DArray smp_scaleMaps;
uniform sampler2DArray smp_distributionMaps;
// -----------------------------------------




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

uniform sampler2D smp_DepthTexture;




// ------------- Donnée de surface de maillage
// Espace homogène de surface : X = T, Y = N, Z = B
uniform sampler2DArray smp_surfaceData;
bool surfaceSpace_IsValid = false;
mat4 surfaceSpace_FromUV(vec2 texCoord)
{
	vec4 positionMap = textureLod(smp_surfaceData,vec3(texCoord,0), 0);
	vec3 normalMap = normalize( textureLod(smp_surfaceData,vec3(texCoord,1), 0).xyz );
	vec3 tangentMap = normalize( textureLod(smp_surfaceData,vec3(texCoord,2), 0).xyz );
	vec3 biTangentMap = normalize( textureLod(smp_surfaceData,vec3(texCoord,3), 0).xyz );

	mat4 surfaceToObjectSpace = mat4(1.0);
	surfaceToObjectSpace[0].xyz = tangentMap;
	surfaceToObjectSpace[1].xyz = normalMap;
	surfaceToObjectSpace[2].xyz = biTangentMap;
	surfaceToObjectSpace[3].xyz = positionMap.xyz;
	surfaceToObjectSpace[3].w = 1.0;

	surfaceSpace_IsValid = (positionMap.w == 1.0);
	return surfaceToObjectSpace;
}
// -----------------------




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


// ----------- Fonction X_t V X
float XtVX(mat2 V,vec2 X)
{
	mat2 t_X = mat2(vec2(X.x,0.0), vec2(X.y,0.0));
	vec2 right = V * X;

	return (t_X * right).x;
}
float XtVX(mat3 V,vec3 X)
{
	mat3 t_X = mat3(vec3(X.x,0.0,0.0), vec3(X.y,0.0,0.0),vec3(X.z,0.0,0.0));
	vec3 right = V * X;
	return (t_X * right).x;
}
float XtVX(mat4 V,vec4 X)
{
	mat4 t_X = mat4(vec4(X.x,0.0,0.0,0.0), vec4(X.y,0.0,0.0,0.0),vec4(X.z,0.0,0.0,0.0),vec4(X.w,0.0,0.0,0.0));
	vec4 right = V * X;
	return (t_X * right).x;
}
// ---------------------------

mat4 move(mat4 space, vec3 translation)
{
    mat4 translationMatrix = mat4(1.0);
    translationMatrix[3].xyz = translation;
    return translationMatrix * space;
}
mat4 rescale(mat4 space, vec3 scaling)
{
    mat4 scalingMatrix = mat4(1.0);
    scalingMatrix[0].x = scaling.x;
    scalingMatrix[1].y = scaling.y;
    scalingMatrix[2].z = scaling.z;
        
    return scalingMatrix * space;
}

mat4 rotate(mat4 space, vec3 around, float angleInRadians)
{
    
    float cosA = cos(angleInRadians);
	float sinA = sin(angleInRadians);

	vec3 axis = normalize(around);
	vec3 temp = (1.0 - cosA) * axis;
	mat4 rotationMatrix = mat4(1.0);
	rotationMatrix[0][0] = cosA + temp[0] * axis[0];
	rotationMatrix[0][1] = 0.0 	+ temp[0] * axis[1] + sinA * axis[2];
	rotationMatrix[0][2] = 0.0 	+ temp[0] * axis[2] - sinA * axis[1];

	rotationMatrix[1][0] = 0.0 	+ temp[1] * axis[0] - sinA * axis[2];
	rotationMatrix[1][1] = cosA + temp[1] * axis[1];
	rotationMatrix[1][2] = 0.0 	+ temp[1] * axis[2] + sinA * axis[0];

	rotationMatrix[2][0] = 0.0 	+ temp[2] * axis[0] + sinA * axis[1];
	rotationMatrix[2][1] = 0.0 	+ temp[2] * axis[1] - sinA * axis[0];
	rotationMatrix[2][2] = cosA + temp[2] * axis[2];

    return rotationMatrix * space;
} 

// Pour filtrer les noyaux, il faut projeter les axes X et Y de la camera dans les sous espaces gaussiens

float computeKernelBase(vec3 kernelPos, 
                        vec3 kernelScale,
                        vec3 evalPos)
{
     mat4 _Vrk = mat4(1.0);
	float distanceEval = 0.0;
	float dVrk = 0.0;
	float g_vrk = 0.0;
   
    
    mat4 shift, rotation, scale, gaussianSpaceMatrix;
    
    // Paramètres de base du noyaux (Idéalement, il me faudrait trouver une matrice qui me réparti la densité
	float power = 10.0;
    mat4 gaussianBase = mat4(1.0); // Gaussienne sphérique
    gaussianBase[0].x = sqrt(PI * power);
    gaussianBase[1].y = sqrt(PI * power);
    gaussianBase[2].z = sqrt(PI * power);
    
    // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(0.0,0.0,0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.7) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0);  
    	

    
    return ( (g_vrk = clamp(power * g_vrk,0.0, 1.0)) < 0.01 ? 0.0 : g_vrk) ;
    
}

float computeCross(vec3 kernelPos, 
                        vec3 kernelScale,
                        vec3 evalPos)
{
     mat4 _Vrk = mat4(1.0);
	float distanceEval = 0.0;
	float dVrk = 0.0;
	float g_vrk = 0.0;
   
    
    mat4 shift, rotation, scale, gaussianSpaceMatrix;
    
    // Paramètres de base du noyaux (Idéalement, il me faudrait trouver une matrice qui me réparti la densité
	float power = 100.0;
    mat4 gaussianBase = mat4(1.0); // Gaussienne sphérique
    gaussianBase[0].x = sqrt(PI * power);
    gaussianBase[1].y = sqrt(PI * power);
    gaussianBase[2].z = sqrt(PI * power);
    
    // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(0.5,0.5,0.5) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.1,0.7,1.0) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0); 
    
    // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(0.5,0.5,0.5) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.7,0.1,1.0) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0); 
    	
    
    return ( (g_vrk = clamp(power * g_vrk,0.0, 1.0)) < 0.01 ? 0.0 : g_vrk) ;
    
}


float computeStars(vec3 kernelPos, 
                        vec3 kernelScale,
                        vec3 evalPos)
{
     mat4 _Vrk = mat4(1.0);
	float distanceEval = 0.0;
	float dVrk = 0.0;
	float g_vrk = 0.0;
   
    
    mat4 shift, rotation, scale, gaussianSpaceMatrix;
    
    // Paramètres de base du noyaux (Idéalement, il me faudrait trouver une matrice qui me réparti la densité
	float power = 1.0;
    mat4 gaussianBase = mat4(1.0); // Gaussienne sphérique
    gaussianBase[0].x = sqrt(PI * power);
    gaussianBase[1].y = sqrt(PI * power);
    gaussianBase[2].z = sqrt(PI * power);
    
    // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(0.0,0.0,0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.05,0.5,1.0) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0); 
    
    // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(0.0,0.0,0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.5,0.05,1.0) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0);
    
    // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(0.0,0.0,0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0), PI_4);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.05,0.5,1.0) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0); 
    
    // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(0.0,0.0,0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0), PI_4);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.5,0.05,1.0) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0);
    	
    
    return ( (g_vrk = clamp(power * g_vrk,0.0, 1.0)) < 0.01 ? 0.0 : g_vrk) ;
    
}

// Test d'un kernel custom 
float computeCustomKernel(vec3 position)
{
    mat4 _Vrk = mat4(1.0);
	float distanceEval = 0.0;
	float dVrk = 0.0;
	float g_vrk = 0.0;
    
    mat4 shift, rotation, scale, gaussianSpaceMatrix;
    
    // Paramètres de base du noyau
	float power = 100.0;
    mat4 gaussianBase = mat4(1.0); // Gaussienne sphérique
    gaussianBase[0].x = sqrt(2.0 * PI * power);
    gaussianBase[1].y = sqrt(2.0 * PI * power);
    gaussianBase[2].z = sqrt(2.0 * PI * power);
    
    // Paramètre de l'espace de la gaussienne : Noyau 1 shifter et tourner
    shift = move(mat4(1.0),vec3(0.5,0.5,0.5));
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),PI_2);
    scale = rescale(mat4(1.0),vec3(0.1,0.8,0.1));
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(position,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval);

    // Paramètre de l'espace de la gaussienne : Noyau 2
    shift = move(mat4(1.0),vec3(0.5,0.5,0.5));
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);
    scale = rescale(mat4(1.0),vec3(0.1,0.8,1.0));
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(position,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval);
    
	
    g_vrk = clamp(power * g_vrk,0.0, 1.0);
    return g_vrk;
}

// Noyaux du donut : réglage scale et power via kernel 0
float computeDonut(vec3 donutPos,vec3 position)
{
    mat4 _Vrk = mat4(1.0);
	float distanceEval = 0.0;
	float dVrk = 0.0;
	float g_vrk = 0.0;
    
    mat4 shift, rotation, scale, gaussianSpaceMatrix;
    
    // Paramètres de base du noyau
	float power = 1.0 / kernels[0].data[1].w;
	vec3 scaleVector = 	kernels[0].data[1].xyz;

    mat4 gaussianBase = mat4(1.0); // Gaussienne sphérique
    gaussianBase[0].x = PI * sqrt(power);
    gaussianBase[1].y = PI * sqrt(power);
    gaussianBase[2].z = PI * sqrt(power);
    
    // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale
    shift = move(mat4(1.0),donutPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);
    scale = rescale(mat4(1.0),vec3(1.0,1.0,1.0) * scaleVector);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
	// Filtrage : Vrk + inverse(J) * inverse(J)t
    distanceEval = (XtVX(_Vrk,vec4(position,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval); // ajouter

    // Paramètre de l'espace de la gaussienne : Noyau 2
    shift = move(mat4(1.0),donutPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);
    scale = rescale(mat4(1.0),vec3(0.9,0.9,1.0) * scaleVector);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(position,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk -=  (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval); // Enlever le noyau central
    
	
    g_vrk = clamp(power * g_vrk,0.0, 1.0);
    return g_vrk;
}

float computeBouton(vec3 kernelPosition, vec3 position)
{
    mat4 _Vrk = mat4(1.0);
	float distanceEval = 0.0;
	float dVrk = 0.0;
	float g_vrk = 0.0;
   
    
    mat4 shift, rotation, scale, gaussianSpaceMatrix;
    
    // Paramètres de base du noyau
	float power = 1.0 / kernels[0].data[1].w;
    vec3 scaleVector = vec3(0.5);
    mat4 gaussianBase = mat4(1.0); // Gaussienne sphérique
    gaussianBase[0].x = sqrt(PI * power);
    gaussianBase[1].y = sqrt(PI * power);
    gaussianBase[2].z = sqrt(PI * power);
    
    // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale
    shift = move(mat4(1.0),kernelPosition);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);
    scale = rescale(mat4(1.0),vec3(1.0,1.0,1.0) * scaleVector);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(position,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0); 
    
    // Paramètre de l'espace de la gaussienne : Noyau 2
    shift = move(mat4(1.0),kernelPosition);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);
    scale = rescale(mat4(1.0),vec3(0.2,0.8,1.0) * scaleVector);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(position,1.0)));
    dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk -=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0 , 1.0); // Enlever le noyau central
    
    // Paramètre de l'espace de la gaussienne : Noyau 3
    shift = move(mat4(1.0),kernelPosition);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);
    scale = rescale(mat4(1.0),vec3(0.8,0.2,1.0) * scaleVector);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(position,1.0)));
    dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk -=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval) , 0.0, 1.0); // Enlever le noyau central
    
	
    g_vrk = clamp(power * g_vrk,0.0, 1.0);
    return g_vrk;
}


float computeGabor(vec3 kernelPos, 
                        vec3 kernelScale,
                        vec3 evalPos)
{
     mat4 _Vrk = mat4(1.0);
	float distanceEval = 0.0;
	float dVrk = 0.0;
	float g_vrk = 0.0;
   
    float F0 = 2.0;
    float w0 = randomIn(-PI_2, PI_2);
    
    mat4 shift, rotation, scale, gaussianSpaceMatrix;
    
    // Paramètres de base du noyaux (Idéalement, il me faudrait trouver une matrice qui me réparti la densité
	float power = 1.0;
    mat4 gaussianBase = mat4(1.0); // Gaussienne sphérique
    gaussianBase[0].x = sqrt(2.0 * PI * power);
    gaussianBase[1].y = sqrt(2.0 * PI * power);
    gaussianBase[2].z = sqrt(2.0 * PI * power);
    
    // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(0.0,0.0,0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.7) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0) * ( cos(PI * 2.0 * F0 * (evalPos.x * cos(w0) + evalPos.y * sin(w0))) ); 
    	
    
    return ( (g_vrk = clamp(power * g_vrk,0.0, 1.0)) < 0.01 ? 0.0 : g_vrk) ;
    
}

float computePatternJMD1(vec3 kernelPos, 
                        vec3 kernelScale,
                        vec3 evalPos)
{
     mat4 _Vrk = mat4(1.0);
	float distanceEval = 0.0;
	float dVrk = 0.0;
	float g_vrk = 0.0;
   
    
    mat4 shift, rotation, scale, gaussianSpaceMatrix;
    
    // Paramètres de base du noyaux (Idéalement, il me faudrait trouver une matrice qui me réparti la densité
	float power = 100.0;
    mat4 gaussianBase = mat4(1.0); // Gaussienne sphérique
    gaussianBase[0].x = sqrt(PI * power);
    gaussianBase[1].y = sqrt(PI * power);
    gaussianBase[2].z = sqrt(PI * power);
        
    // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(0.2,-0.2,0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),PI_4);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.05,0.5,0.5) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0); 
    
    // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(0.2,0.2,0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),PI_4 + PI_2);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.05,0.5,0.5) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0); 
    	
    
    return ( (g_vrk = clamp(power * g_vrk,0.0, 1.0)) < 0.01 ? 0.0 : g_vrk) ;
    
}

float computePatternJMD2(vec3 kernelPos, 
                        vec3 kernelScale,
                        vec3 evalPos)
{
    mat4 _Vrk = mat4(1.0);
	float distanceEval = 0.0;
	float dVrk = 0.0;
	float g_vrk = 0.0;
   
    
    mat4 shift, rotation, scale, gaussianSpaceMatrix;
    
    // Paramètres de base du noyaux (Idéalement, il me faudrait trouver une matrice qui me réparti la densité
	float power = 5.0;
    mat4 gaussianBase = mat4(1.0); // Gaussienne sphérique
    gaussianBase[0].x = sqrt(PI * power);
    gaussianBase[1].y = sqrt(PI * power);
    gaussianBase[2].z = sqrt(PI * power);
    
	// A prendre en compte : en utilisant un éditeur : la matrice de transformation du noyau serait très facilement paramétrable
	// Il suffirait d'exprimer

    // Paramètre de l'espace de la gaussienne : Noyau 1
    shift = move(mat4(1.0),vec3(0.0,0.3,0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),PI_4);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.05,0.5,0.5) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0);
    
    // Paramètre de l'espace de la gaussienne : Noyau 1
    shift = move(mat4(1.0),vec3(0.3,0.0,0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),PI_4);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.5,0.05,0.5) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0);
    	
    
    return ( (g_vrk = clamp(power * g_vrk,0.0, 1.0)) < 0.01 ? 0.0 : g_vrk) ;
    
}

float computePatternCurve(vec3 kernelPos, 
                        vec3 kernelScale,
                        vec3 evalPos)
{
     mat4 _Vrk = mat4(1.0);
	float distanceEval = 0.0;
	float dVrk = 0.0;
	float g_vrk = 0.0;
   
    
    mat4 shift, rotation, scale, gaussianSpaceMatrix;
    
    // Paramètres de base du noyaux (Idéalement, il me faudrait trouver une matrice qui me réparti la densité
	float power = 5.0;
    mat4 gaussianBase = mat4(1.0); // Gaussienne sphérique
    gaussianBase[0].x = sqrt(PI * power);
    gaussianBase[1].y = sqrt(PI * power);
    gaussianBase[2].z = sqrt(PI * power);
    
     // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(sign(sin(evalPos.y - kernelPos.y)) * 1.5 * pow(sin(evalPos.y - kernelPos.y),2.0),0.0,0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.05,0.5,0.5) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0);
    	
    
    return ( (g_vrk = clamp(power * g_vrk,0.0, 1.0)) < 0.01 ? 0.0 : g_vrk) ;
    
}

float computePatternCurve2(vec3 kernelPos, 
                        vec3 kernelScale,
                        vec3 evalPos)
{
     mat4 _Vrk = mat4(1.0);
	float distanceEval = 0.0;
	float dVrk = 0.0;
	float g_vrk = 0.0;
   
    
    mat4 shift, rotation, scale, gaussianSpaceMatrix;
    
    // Paramètres de base du noyaux (Idéalement, il me faudrait trouver une matrice qui me réparti la densité
	float power = 5.0;
    mat4 gaussianBase = mat4(1.0); // Gaussienne sphérique
    gaussianBase[0].x = sqrt(PI * power);
    gaussianBase[1].y = sqrt(PI * power);
    gaussianBase[2].z = sqrt(PI * power);
    
     // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(sign(sin(evalPos.y - kernelPos.y)) * 1.5 * pow(sin(evalPos.y - kernelPos.y),2.0),0.0,0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.05,0.5,0.5) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0);
    
     // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(- sign(sin(evalPos.y - kernelPos.y)) * 1.5 * pow(sin(evalPos.y - kernelPos.y),2.0),0.0,0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.05,0.5,0.5) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0);
    	
    
    return ( (g_vrk = clamp(power * g_vrk,0.0, 1.0)) < 0.01 ? 0.0 : g_vrk) ;
    
}

float computePatternCurve3(vec3 kernelPos, 
                        vec3 kernelScale,
                        vec3 evalPos)
{
     mat4 _Vrk = mat4(1.0);
	float distanceEval = 0.0;
	float dVrk = 0.0;
	float g_vrk = 0.0;
   
    
    mat4 shift, rotation, scale, gaussianSpaceMatrix;
    
    // Paramètres de base du noyaux (Idéalement, il me faudrait trouver une matrice qui me réparti la densité
	float power = 1.0;
    mat4 gaussianBase = mat4(1.0); // Gaussienne sphérique
    gaussianBase[0].x = sqrt(PI * power);
    gaussianBase[1].y = sqrt(PI * power);
    gaussianBase[2].z = sqrt(PI * power);
    
     // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(sign(sin(evalPos.y - kernelPos.y)) * pow(sin(evalPos.y - kernelPos.y),2.0),0.0,0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.1,0.5,0.5) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0);
    
     // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(- sign(sin(evalPos.y - kernelPos.y)) * pow(sin(evalPos.y - kernelPos.y),2.0),0.0,0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.1,0.5,0.5) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (XtVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0);
    	
    
    return ( (g_vrk = clamp(power * g_vrk,0.0, 1.0)) < 0.01 ? 0.0 : g_vrk) ;
    
}

// Debut du fragment shader
struct Vertex{
	vec4 Position;
	vec4 Normal;
	vec4 Texture_Coordinates;
};
in Vertex _vertex;
out vec4 outColor;

void main()
{
	outColor = vec4(0.0);
	vec2 uv = _vertex.Texture_Coordinates.xy;
	// PARAMETRES
	int nbKernelPerCell = int(kernels[0].data[2].z);
	int nbSlice = 1;

	vec2 cellHalfSize = 0.5 / iv3_VoxelGrid_subdiv.xy;

	// Position du point de la surface dans la grille
	vec2 pos = _vertex.Texture_Coordinates.xy * iv3_VoxelGrid_subdiv.xy;
	ivec2 currentCell = ivec2(floor(pos)); 
	
	float value = 0.0;
	for(int y_i = currentCell.y - 1; y_i <= currentCell.y + 1; y_i++)
	for(int x_i = currentCell.x - 1; x_i <= currentCell.x + 1; x_i++)
	{
		ivec2 evalCell = ivec2(x_i,y_i);
		initRandom(evalCell);
		for(int k_i = 0; k_i < nbKernelPerCell ; k_i++)
		{
			// Kernel position in the cell
			//vec3 kernelPosition = vec3(randomInFrame(vec2(0.5), vec2(dMinFactor) ,vec2(dMaxFactor) ),0.0);
			vec3 kernelPosition = randomInPartialCylinder(vec3(0.5,0.5,0.0), (dMinFactor) ,(dMaxFactor), 0.0,0.0);
			
			// pixel position in the cell
			vec3 evalPos = vec3(pos - evalCell,0.0);

			value += computePatternCurve1(kernelPosition, randomIn(vec3(0.5),vec3(1.5)) ,evalPos);
			//value += computeKernelBase(kernelPosition, vec3(1.0) ,evalPos);
		}
	}

	value = clamp(value,0.0,1.0);
	outColor = vec4(value);
}
