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

//uniform sampler2D smp_depthBuffer;





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


in vec4 v_position;
in vec3 v_normal;
in vec3 v_textureCoords;
in vec3 surface_position;
in vec3 surface_normale;
in mat4 kernelToObjectSpace;
flat in int kernelTextureID;
in kernel computeKernel;

in float gridHeight;

flat in int v_invalidKernel;

in vec4 kernelBaseColor;

layout (location = 0) out vec4 Color;

// Passe d'accumulation pondéré par OIT
void main()
{
	if(v_invalidKernel > 0.1) discard;

	//float z_c = gl_FragCoord.z;
	//float z_b = texelFetch(smp_depthBuffer,ivec2(gl_FragCoord.xy),0).x;
						
	
	// Pour le splatting via rasterization : 
	// les coordonnées de textures du fragment corresponde à la déprojection du fragment dans l'espace du noyau
	vec3 kernelFragmentPosition = v_textureCoords;
	vec3 kernelFragmentRight = v_textureCoords + dFdx(kernelFragmentPosition);
	vec3 kernelFragmentUp = v_textureCoords + dFdy(kernelFragmentPosition);
	kernelFragmentPosition.z = 0.0;
	

	// sample dans l'espace camera pour calcul de la profondeur
	vec3 object_point = (kernelToObjectSpace * vec4(kernelFragmentPosition,1.0)).xyz; 
	vec4 pointInCamera = objectToCamera * vec4( object_point , 1.0);
	pointInCamera.z = abs(pointInCamera.z);

	
	vec3 VV = (object_point.xyz - surface_position.xyz);
	float height = dot(normalize(surface_normale.xyz),VV);
	
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
	g_vrk = clamp( computeKernel.data[1].w * g_vrk,0.0, 1.0);

	// adaptation du calcul pour choix de resultat
	vec2 kernelTextureCoordinates = ((kernelFragmentPosition.xy) + vec2(0.5,0)).yx;
	//kernelTextureCoordinates.y -= 0.5;
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
	vec3 worldGroundNormal =  (objectToWorld * vec4(surface_normale.xyz,0.0)).xyz;

	bool isAValidSample = kernelFragmentPosition.y > 0  &&  g_vrk > 0.01;
	if( !isAValidSample ) discard;
	
	
	// -- texturing simple :
	bool isInTextureSpace = all( bvec2( all(greaterThan(kernelTextureCoordinates,vec2(0.01))) , all(lessThan(kernelTextureCoordinates,vec2(0.99)))) );

	//kernelColor = g_vrk * vec4(0.05,0.5,0,1);
	vec4 kernelColor = isInTextureSpace ? 
		textureGrad(smp_models,vec3(kernelTextureCoordinates, kernelTextureID ), kernelTextureDx, kernelTextureDy ) : 
		vec4(0.0);
	
		
		
	vec4 kernelContrib;
	float VolumetricOcclusionFactor = height / (gridHeight * (computeKernel.data[6].w));
	float bendingTest = abs(dot(normalize(worldGroundNormal),normalize(worldNormal)));
	
	//if(bendingTest > 0.5) VolumetricOcclusionFactor = 0.5;
	

	//kernelContrib = g_vrk * vec4(0.05,0.5,0,1);;
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
	//kernelContrib = vec4(kernelBaseColor.rgb,g_vrk);
	//kernelContrib = kernelColor;

	float oitWeight = weightFunction(pointInCamera.z);
	
	float newAlpha = kernelContrib.a * oitWeight;
	// Test

	
	// Opti a la con
	bool isContributive = kernelContrib.a > 0.001;
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
	
	//if(kernelContrib.a < 0.01) discard;
	//Color = kernelContrib * kernelContrib.a;
	Color = vec4(kernelContrib.rgb * newAlpha , newAlpha);// * kernelColor.w;
	
}

/* backup

	// Supprimer les fragments > au depthBuffer
	//if(z_c > z_b) discard;

	float zNear = 0.1f;
	float zFar = 500.0f;

	float z_n = 2.0 * z_c - 1.0;
    float z_e = 2.0 * zNear * zFar / (zFar + zNear - z_n * (zFar - zNear));

	// Texture coord = fragment in the kernel space
	vec4 kernelColor = texture(smp_models,vec3(v_textureCoords.yx,0));

	kernelColor.w = v_textureCoords.x >= 0.99 ? 0.0f : kernelColor.w;
	kernelColor.w = v_textureCoords.x <= 0.01 ? 0.0f : kernelColor.w;
	kernelColor.w = v_textureCoords.y >= 0.99 ? 0.0f : kernelColor.w;
	kernelColor.w = v_textureCoords.y <= 0.01 ? 0.0f : kernelColor.w;

	float oitWeight = kernelColor.w * macGuireEq8(z_e);
	// Dans RGB, on accumul alpha * color,
*/