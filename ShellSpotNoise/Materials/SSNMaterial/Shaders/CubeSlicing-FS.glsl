#version 430
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Common"
#include "/Materials/Common/Lighting/Lighting"				// Pseudo Random Number Generator						
#line 6

// constantes
#ifndef PI 
	#define PI 3.14159265359
	#define PI_2 1.57079632679
	#define PI_4 0.78539816339
#endif

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



layout(std140) uniform CPU
{
	mat4 objectToScreen; 	// MVP
	mat4 objectToCamera; 	// MV
	mat4 objectToWorld;		// M
};
uniform sampler2DArray smp_models;

uniform sampler2D smp_depthBuffer;


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
	// Axis in screen space
	vec2 axeX = XScreen - PScreen;
	vec2 axeY = YScreen - PScreen;
	mat3 XYScreenSpace = mat3(	vec3(axeX,0),
								vec3(axeY,0),
								vec3(PScreen,1));
	return XYScreenSpace;
}
// ---------------------------------------------------------------------

// ---------------------- Operation XtVX
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

float matmul_XVX(mat4 V,vec4 X)
{
	mat4 t_X = mat4(vec4(X.x,0.0,0.0,0.0), vec4(X.y,0.0,0.0,0.0),vec4(X.z,0.0,0.0,0.0),vec4(X.w,0.0,0.0,0.0));
	vec4 right = V * X;
	return (t_X * right).x;
}
// ---------------------- Operation XtVX END

// -------------------- Matrix Transformations
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
// -------------------- Matrix Transformations END

/**
*	Order independent transparency weighting function
*/
float weightFunction(float depth)
{
	//return max(1.0 , 1.0 + exp(-depth/10));
	return clamp(100.0 / max(1.0 + exp(1.0+max(depth,0.01)),0.1),1.0,3000.0);
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


float kernelEvaluation(
	in kernel config, 
	vec3 position
	)
{

	//// jacobienne du point d'évaluation calculé dans l'espace écran
	//vec2 dx = kernelFragmentRight.xy - position.xy;
	//vec2 dy = kernelFragmentUp.xy - position.xy;
	//mat2 J = mat2(dx,dy);
	// Gaussienne dans l'espace du noyau (a paramétrer avec rx et ry)
	mat3 Vrk = mat3(	vec3(config.data[1].x,0,0),
						vec3(0,config.data[1].y,0),
						vec3(0,0,config.data[1].z)
						);
	// filtrage : application de la jacobienne
	//Vrk += (J * transpose(J));
	mat3 _Vrk = inverse(Vrk);
	float dVrk = sqrt(determinant(Vrk));
	float distanceEval = matmul_XVX(_Vrk,position);
	float g_vrk =  (1.0 / (2.0 * PI * dVrk)) * exp(-0.5 * distanceEval);
	g_vrk = clamp( config.data[1].w * g_vrk,0.0, 1.0);


	return g_vrk;
}

float filteredKernelEvaluation(
	in kernel config, 
	vec3 position,
	mat3 jacobian
	)
{

	// jacobienne du point d'évaluation calculé dans l'espace écran
	mat3 J = jacobian;
	// Gaussienne dans l'espace du noyau (a paramétrer avec rx et ry)
	mat3 Vrk = mat3(	vec3(config.data[1].x,0,0),
						vec3(0,config.data[1].y,0),
						vec3(0,0,config.data[1].z)
						);
	// filtrage : application de la jacobienne
	Vrk += (J * transpose(J));
	mat3 _Vrk = inverse(Vrk);
	float dVrk = sqrt(determinant(Vrk));
	float distanceEval = matmul_XVX(_Vrk,position);
	float g_vrk =  (1.0 / (2.0 * PI * dVrk)) * exp(-0.5 * distanceEval);
	g_vrk = clamp( config.data[1].w * g_vrk,0.0, 1.0);


	return g_vrk;
}


in vec4 v_position;
in vec3 v_normal;
in vec3 v_textureCoords;
in vec3 v_fragCoord;
in vec3 surface_position;
in vec3 surface_normale;
in mat4 kernelToObjectSpace;
flat in int kernelTextureID;
in kernel computeKernel;

in float gridHeight;

flat in int v_invalidKernel;

in vec4 kernelBaseColor;

layout (location = 0) out vec4 Color;

void main_test()
{
	if(v_invalidKernel > 0.1) discard;

	vec2 screenFrag = (v_fragCoord.xy*0.5) + 0.5;
	Color = vec4(v_fragCoord.xy,0,1);
}



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
	float power = 100.0;
    mat4 gaussianBase = mat4(1.0); // Gaussienne sphérique
    gaussianBase[0].x = sqrt(PI * power);
    gaussianBase[1].y = sqrt(PI * power);
    gaussianBase[2].z = sqrt(PI * power);
    
    // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);	// Todo 
    scale = rescale(mat4(1.0),vec3(1.0) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (matmul_XVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0); 
    	
    
    return ( (g_vrk = clamp(power * g_vrk,0.0, 1.0)) < 0.01 ? 0.0 : g_vrk) ;
    
}

float computeStars(
	vec3 kernelPos, 
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
    shift = move(mat4(1.0),vec3(0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.1,0.5,0.1) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (matmul_XVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0); 
    
    // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.5,0.1,0.1) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (matmul_XVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0);

	// Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),PI_4);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.1,0.5,0.1) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (matmul_XVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0); 
    
    // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),PI_4);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.5,0.1,0.1) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (matmul_XVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0);


	// Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,1.0,0.0),PI_4);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.1,0.5,0.1) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (matmul_XVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0); 
    
    // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,1.0,0.0),PI_4);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.5,0.1,0.1) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (matmul_XVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0);

	// Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(1.0,0.0,0.0),PI_4);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.1,0.5,0.1) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (matmul_XVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0); 
    
    // Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
    shift = move(mat4(1.0),vec3(0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(1.0,0.0,0.0),PI_4);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.5,0.1,0.1) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (matmul_XVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0);
    

    	
    
    return ( (g_vrk = clamp(power * g_vrk,0.0, 1.0)) < 0.01 ? 0.0 : g_vrk) ;
    
}

// test à la nawak
float computeNawak(
	vec3 kernelPos, 
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
    shift = move(mat4(1.0),vec3(0.0) + kernelPos);
    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0),0.0);	// Todo 
    scale = rescale(mat4(1.0),vec3(0.1,0.1,0.1) * kernelScale);
    gaussianSpaceMatrix = shift * rotation * scale;  
    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
	_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
    distanceEval = (matmul_XVX(_Vrk,vec4(evalPos,1.0)));
	dVrk = sqrt(determinant(gaussianSpaceMatrix));
	g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0); 
    
    float nbPique = 4.0;
    for(int i = 0; i < nbPique ; i++)
    {
    	// Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
	    shift = move(mat4(1.0),vec3(0.0) + kernelPos);
	    rotation = rotate(mat4(1.0),vec3(0.0,0.0,1.0), i * PI_2 / nbPique);	// Todo 
	    scale = rescale(mat4(1.0),vec3(0.05,0.5,0.05) * kernelScale);
	    gaussianSpaceMatrix = shift * rotation * scale;  
	    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
		_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
	    distanceEval = (matmul_XVX(_Vrk,vec4(evalPos,1.0)));
		dVrk = sqrt(determinant(gaussianSpaceMatrix));
		g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0);

		// Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
	    shift = move(mat4(1.0),vec3(0.0) + kernelPos);
	    rotation = rotate(mat4(1.0),vec3(0.0,1.0,0.0), i * PI_2 / nbPique);	// Todo 
	    scale = rescale(mat4(1.0),vec3(0.05,0.5,0.05) * kernelScale);
	    gaussianSpaceMatrix = shift * rotation * scale;  
	    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
		_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
	    distanceEval = (matmul_XVX(_Vrk,vec4(evalPos,1.0)));
		dVrk = sqrt(determinant(gaussianSpaceMatrix));
		g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0); 

		// Paramètre de l'espace de la gaussienne : Noyau 1 (gaussienne normale shifter de 0.5 en xyz)
	    shift = move(mat4(1.0),vec3(0.0) + kernelPos);
	    rotation = rotate(mat4(1.0),vec3(1.0,0.0,0.0), i * PI_2 / nbPique);	// Todo 
	    scale = rescale(mat4(1.0),vec3(0.05,0.5,0.05) * kernelScale);
	    gaussianSpaceMatrix = shift * rotation * scale;  
	    gaussianSpaceMatrix = inverse(gaussianSpaceMatrix);
		_Vrk = transpose(gaussianSpaceMatrix) * gaussianBase * gaussianSpaceMatrix;
	    distanceEval = (matmul_XVX(_Vrk,vec4(evalPos,1.0)));
		dVrk = sqrt(determinant(gaussianSpaceMatrix));
		g_vrk +=  clamp( (dVrk / (2.0 * PI)) * exp(-0.5 * distanceEval), 0.0, 1.0); 
    } 

    	
    
    return ( (g_vrk = clamp(power * g_vrk,0.0, 1.0)) < 0.01 ? 0.0 : g_vrk) ;
    
}

// Passe d'accumulation pondéré par OIT -- version voxel
void main()
{
	if(v_invalidKernel > 0.1) discard;

	mat4 objectToKernelSpace = inverse(kernelToObjectSpace);	

	mat4 cameraToObject = inverse(objectToCamera);

	vec3 kernelCameraDirInObjet = normalize(kernelToObjectSpace[3].xyz - cameraToObject[3].xyz);
	
	float sommeAlphaS = 0.0;
	vec3 sommePos = vec3(0.0);
	vec3 sommeNormal = vec3(0.0);

	// Direction kernel->camera normalized dans l'espace noyau puis retransformer dans l'espace objet
	kernelCameraDirInObjet = (kernelToObjectSpace * vec4(normalize( (objectToKernelSpace * vec4(kernelCameraDirInObjet,0.0)).xyz),0)).xyz;
	vec3 kernelFragmentPosition;
	int nbSlice = 10;
	for(int sl = 1; sl <= nbSlice && sommeAlphaS < 1.0; ++sl)
	{
		vec3 currentSlicePos = kernelToObjectSpace[3].xyz - ( 1.0 - float(sl) / (float(nbSlice + 1) * 0.5)) * (kernelCameraDirInObjet) ;
		// une slice (orthognal a la caméra) correspond à un espace 2D
		mat3 pixelToSliceSpace = project2DSpaceToScreen(currentSlicePos ,cameraToObject[0].xyz , cameraToObject[1].xyz, objectToScreen);
		pixelToSliceSpace = inverse(pixelToSliceSpace);

		// Position du pixel courant dans la slice du noyau (espace objet)
		vec3 pixelPositionInSlice = pixelToSliceSpace * vec3(v_fragCoord.xy,1.0);
		pixelPositionInSlice = currentSlicePos + pixelPositionInSlice.x * cameraToObject[0].xyz + pixelPositionInSlice.y * cameraToObject[1].xyz;
		
		// Echantillon du noyau sélectionné
		kernelFragmentPosition = (objectToKernelSpace * vec4(pixelPositionInSlice,1.0)).xyz;

		// information pour calcul dans la surcouche
		vec3 object_point = pixelPositionInSlice;
		vec3 VV = (object_point.xyz - surface_position.xyz);
		float height = dot(normalize(surface_normale.xyz),VV);

		// sample dans l'espace camera pour calcul de la profondeur 
		vec4 pointInCamera = objectToCamera * vec4( object_point , 1.0);
		pointInCamera.z = abs(pointInCamera.z);
		
		// fin du calcul si l'échantillon courant est après le depth buffer
		float depthTest = pointInCamera.z;
		vec4 z_b = textureLod(smp_depthBuffer,gl_FragCoord.xy / vec2(1280.0,720.0),0);
		if(z_b.w > 0.0f && depthTest > z_b.x) break;
		

		//float g_vrk = kernelEvaluation( computeKernel, kernelFragmentPosition );
		
		float g_vrk = computeStars(vec3(0.0), vec3(1.0) ,kernelFragmentPosition);
		//vec4 kernelColor = vec4(0.4,0.8,0.3,sommeAlphaS);


		// test avec 2em noyau :
		//g_vrk = computeKernel(pixelPositionInKernel ,  kernels[i].data[1].xzy, kernels[i].data[1].w );
		//sommeAlphaS += g_vrk;
		//sommePos += pixelPositionInSlice * g_vrk;
		//sommeNormal += g_vrk * normalize(( kernelToObjectSpace * vec4(pixelPositionInKernel,0.0)).xyz);

		// Pour OIT
		//float oitWeight = weightFunction(pointInCamera.z);
		//oitWeight *= (1.0 + pow( (height / gridHeight)*100,4.0 ) );
		//float newAlpha = g_vrk * oitWeight;

		sommeAlphaS += g_vrk;
		sommePos += pixelPositionInSlice * g_vrk;
		sommeNormal += g_vrk * normalize(( kernelToObjectSpace * vec4(kernelFragmentPosition,0.0)).xyz);

	}

	vec4 kernelColor = vec4(0.0);
			
	// Changement de couleur linéaire
	//kernelColor = mix( vec4(0.8,0.6,0.1,sommeAlphaS) , vec4(0.6,0.1,0.8,sommeAlphaS) ,transition);
	kernelColor = vec4(0.4,0.8,0.3,1.0);
	//if(i == 0)	kernelColor = vec4(0.4,0.8,0.3,sommeAlphaS);
	//if(i == 1)	kernelColor = vec4(0.2,0.8,1.0,sommeAlphaS);
	//if(i == 2)	kernelColor = vec4(1.0,1.0,0.0,sommeAlphaS);

	// Reconstruction de surface seulement si l'on a atteint la quantité de matière maximum
	if(sommeAlphaS >= 1.0)
	{
		sommePos /= sommeAlphaS;
		sommeNormal /= sommeAlphaS;
	}
	else discard;
	//if(sommeAlphaS < 0.01) discard;


	// information pour calcul dans la surcouche
	vec3 object_point = sommePos;
	vec3 VV = (object_point.xyz - surface_position.xyz);
	float height = dot(normalize(surface_normale.xyz),VV);

	// sample dans l'espace camera pour calcul de la profondeur 
	vec4 pointInCamera = objectToCamera * vec4( object_point , 1.0);
	pointInCamera.z = abs(pointInCamera.z);

	// fin du calcul si l'échantillon courant est après le depth buffer
	float depthTest = pointInCamera.z;
	vec4 z_b = textureLod(smp_depthBuffer,gl_FragCoord.xy / vec2(1280.0,720.0),0);
	if(z_b.w > 0.0f && depthTest > z_b.x) discard;

	
	
	vec3 worldPosition = (objectToWorld * vec4(sommePos,1.0)).xyz;
	vec3 worldNormal = (objectToWorld * vec4(sommeNormal,1.0)).xyz;
	vec3 worldGroundNormal = (objectToWorld * vec4(surface_normale,1.0)).xyz;

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
	//
	//	float oitWeight = weightFunction(pointInCamera.z);
	//	oitWeight *= (1.0 + pow( (height / gridHeight)*100,4.0 ) );
	//	float newAlpha = kernelContrib.a * oitWeight;
	kernelContrib = addPhong(worldPosition, worldNormal,vec4(0.25,0.25,0.5,1.0), vec4(0.25,0.25,0.5,1.0),vec4(1.0),vec4(0.1,0.5,0.4,64));
	kernelContrib.w = 1.0;
		
	if(kernelContrib.a < 0.01) discard;
	//kernelContrib = vec4(sommeNormal,1.0);
	//Color = vec4(sommeAlphaS);
	Color = kernelContrib;//vec4(kernelColor.rgb * newAlpha , newAlpha);// * kernelColor.w;
	
}
