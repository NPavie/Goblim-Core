#version 430
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Kernels"
#line 5

layout(std140) uniform CPU
{
	mat4 objectToScreen; 	// MVP
	mat4 objectToCamera; 	// MV
	mat4 objectToWorld;		// M
};
uniform sampler2DArray smp_models;
uniform sampler2D smp_depthBuffer;


layout (location = 0) out vec4 Color;


float kernelEvaluation(
	in kernelProfile config, 
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
	float distanceEval = XtVY(position,_Vrk,position);
	float g_vrk =  (1.0 / (2.0 * PI * dVrk)) * exp(-0.5 * distanceEval);
	g_vrk = clamp( config.data[1].w * g_vrk,0.0, 1.0);


	return g_vrk;
}

float filteredKernelEvaluation(
	in kernelProfile config, 
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
	float distanceEval = XtVY(position,_Vrk,position);
	float g_vrk =  (1.0 / (2.0 * PI * dVrk)) * exp(-0.5 * distanceEval);
	g_vrk = clamp( config.data[1].w * g_vrk,0.0, 1.0);


	return g_vrk;
}


// Output : kernel informations
in kernelProfile	surfaceKernel_profile;
flat in int surfaceKernel_textureID;
//in mat4 surfaceKernel_toObjectSpace;
//in mat4 surfaceKernel_toWorldSpace;
in mat4 surfaceKernel_toViewSpace;
//in mat4 surfaceKernel_fromObjectSpace;
//in mat4 surfaceKernel_fromWorldSpace;
//in mat4 surfaceKernel_fromViewSpace;

in float surfacePoint_shellHeight;
in vec3 surfacePoint_position;
in vec3 surfacePoint_normale;
in vec3 surfacePoint_tangente;

flat in int 		isAValidKernel;
in vec4 kernelEvaluationPoint;

void main()
{
	if( isAValidKernel > 0.1 ) discard;
	
	vec3 intersectedPosition = vec3(0.5);
	vec3 intersectedNormal = normalize(vec3(1.0));
	
	bool isIntersected = intersection_quadraticKernel(
							vec3(0.0), 
						    surfaceKernel_profile.data[1].xyz,
						    surfaceKernel_toViewSpace * kernelEvaluationPoint,
						    surfaceKernel_toViewSpace,
						    intersectedPosition,
						    intersectedNormal
							);
	

	if(isIntersected == false) discard;
	


	Color = vec4(normalize(intersectedNormal),1.0);
}









//in vec4 v_position;
//in vec3 v_normal;
//in vec3 v_textureCoords;
//
//in vec3 v_CameraSpaceCoord;
//
//in vec3 surface_position;
//in vec3 surface_normale;
//in mat4 kernelToObjectSpace;
//flat in int kernelTextureID;
//in kernelProfile computeKernel;
//
//in float gridHeight;
//
//flat in int v_invalidKernel;
//
//in vec4 kernelBaseColor;
//
//
//
//
//void main()
//{
//	if( v_invalidKernel > 0.1 ) discard;
//
//	mat4 KernelToCameraSpace = objectToCamera * kernelToObjectSpace;
//	
//	vec3 intersectedPosition = vec3(0.5);
//	vec3 intersectedNormal = normalize(vec3(1.0));
//	
//	bool isIntersected = intersection_quadraticKernel(
//							vec3(0.0), 
//						    computeKernel.data[1].xyz,
//						    KernelToCameraSpace * v_position,
//						    KernelToCameraSpace,
//						    intersectedPosition,
//						    intersectedNormal
//							);
//	
//
//	if(isIntersected == false) discard;
//	
//
//
//	Color = vec4(normalize(intersectedNormal),1.0);
//}



// OLD --- Slicing de noyau
//void main_slicing()
//{
//	if(v_invalidKernel > 0.1) discard;
//
//	mat4 objectToKernelSpace = inverse(kernelToObjectSpace);	
//
//	mat4 cameraToObject = inverse(objectToCamera);
//
//	vec3 kernelCameraDirInObjet = normalize(kernelToObjectSpace[3].xyz - cameraToObject[3].xyz);
//	
//	float sommeAlphaS = 0.0;
//	vec3 sommePos = vec3(0.0);
//	vec3 sommeNormal = vec3(0.0);
//
//	// Direction kernel->camera normalized dans l'espace noyau puis retransformer dans l'espace objet
//	kernelCameraDirInObjet = (kernelToObjectSpace * vec4(normalize( (objectToKernelSpace * vec4(kernelCameraDirInObjet,0.0)).xyz),0)).xyz;
//	vec3 kernelFragmentPosition;
//	int nbSlice = 10;
//	for(int sl = 1; sl <= nbSlice && sommeAlphaS < 1.0; ++sl)
//	{
//		vec3 currentSlicePos = kernelToObjectSpace[3].xyz - ( 1.0 - float(sl) / (float(nbSlice + 1) * 0.5)) * (kernelCameraDirInObjet) ;
//		// une slice (orthognal a la caméra) correspond à un espace 2D
//		mat3 pixelToSliceSpace = project2DSpaceToScreen(currentSlicePos ,cameraToObject[0].xyz , cameraToObject[1].xyz, objectToScreen);
//		pixelToSliceSpace = inverse(pixelToSliceSpace);
//
//		// Position du pixel courant dans la slice du noyau (espace objet)
//		vec3 pixelPositionInSlice = pixelToSliceSpace * vec3(v_fragCoord.xy,1.0);
//		pixelPositionInSlice = currentSlicePos + pixelPositionInSlice.x * cameraToObject[0].xyz + pixelPositionInSlice.y * cameraToObject[1].xyz;
//		
//		// Echantillon du noyau sélectionné
//		kernelFragmentPosition = (objectToKernelSpace * vec4(pixelPositionInSlice,1.0)).xyz;
//
//		// information pour calcul dans la surcouche
//		vec3 object_point = pixelPositionInSlice;
//		vec3 VV = (object_point.xyz - surface_position.xyz);
//		float height = dot(normalize(surface_normale.xyz),VV);
//
//		// sample dans l'espace camera pour calcul de la profondeur 
//		vec4 pointInCamera = objectToCamera * vec4( object_point , 1.0);
//		pointInCamera.z = abs(pointInCamera.z);
//		
//		float zBufferDepth = textureLod(smp_depthBuffer,gl_FragCoord.xy / vec2(1920.0,1080.0),0).x;
//		if(-pointInCamera.z > zBufferDepth) break;
//		
//
//		//float g_vrk = kernelEvaluation( computeKernel, kernelFragmentPosition );
//		
//		float g_vrk = computeStars(vec3(0.0), vec3(1.0) ,kernelFragmentPosition);
//		//vec4 kernelColor = vec4(0.4,0.8,0.3,sommeAlphaS);
//
//
//		// test avec 2em noyau :
//		//g_vrk = computeKernel(pixelPositionInKernel ,  kernels[i].data[1].xzy, kernels[i].data[1].w );
//		//sommeAlphaS += g_vrk;
//		//sommePos += pixelPositionInSlice * g_vrk;
//		//sommeNormal += g_vrk * normalize(( kernelToObjectSpace * vec4(pixelPositionInKernel,0.0)).xyz);
//
//		// Pour OIT
//		//float oitWeight = weightFunction(pointInCamera.z);
//		//oitWeight *= (1.0 + pow( (height / gridHeight)*100,4.0 ) );
//		//float newAlpha = g_vrk * oitWeight;
//
//		sommeAlphaS += g_vrk;
//		sommePos += pixelPositionInSlice * g_vrk;
//		sommeNormal += g_vrk * normalize(( kernelToObjectSpace * vec4(kernelFragmentPosition,0.0)).xyz);
//
//	}
//
//	vec4 kernelColor = vec4(0.0);
//			
//	// Changement de couleur linéaire
//	//kernelColor = mix( vec4(0.8,0.6,0.1,sommeAlphaS) , vec4(0.6,0.1,0.8,sommeAlphaS) ,transition);
//	kernelColor = vec4(0.4,0.8,0.3,1.0);
//	//if(i == 0)	kernelColor = vec4(0.4,0.8,0.3,sommeAlphaS);
//	//if(i == 1)	kernelColor = vec4(0.2,0.8,1.0,sommeAlphaS);
//	//if(i == 2)	kernelColor = vec4(1.0,1.0,0.0,sommeAlphaS);
//
//	// Reconstruction de surface seulement si l'on a atteint la quantité de matière maximum
//	if(sommeAlphaS >= 1.0)
//	{
//		sommePos /= sommeAlphaS;
//		sommeNormal /= sommeAlphaS;
//	}
//	else discard;
//	//if(sommeAlphaS < 0.01) discard;
//
//
//	// information pour calcul dans la surcouche
//	vec3 object_point = sommePos;
//	vec3 VV = (object_point.xyz - surface_position.xyz);
//	float height = dot(normalize(surface_normale.xyz),VV);
//
//	// sample dans l'espace camera pour calcul de la profondeur 
//	vec4 pointInCamera = objectToCamera * vec4( object_point , 1.0);
//	
//
//	// fin du calcul si l'échantillon courant est après le depth buffer
//
//	float zBufferDepth = textureLod(smp_depthBuffer,gl_FragCoord.xy / vec2(1920.0,1080.0),0).x;
//	if(-pointInCamera.z > zBufferDepth) discard;
//
//	
//	
//	vec3 worldPosition = (objectToWorld * vec4(sommePos,1.0)).xyz;
//	vec3 worldNormal = (objectToWorld * vec4(sommeNormal,1.0)).xyz;
//	vec3 worldGroundNormal = (objectToWorld * vec4(surface_normale,1.0)).xyz;
//
//	vec4 kernelContrib;
//	float VolumetricOcclusionFactor = height / (gridHeight * (computeKernel.data[6].w));
//	float bendingTest = abs(dot(normalize(worldGroundNormal),normalize(worldNormal)));
//	
//	//if(bendingTest > 0.5) VolumetricOcclusionFactor = 0.5;
//	
//
//	//kernelContrib = vec4(kernelBaseColor.rgb,g_vrk);
//	kernelContrib = addBoulanger3(	
//									worldPosition,
//       								kernelColor,
//       								worldNormal,
//       								worldGroundNormal,
//       								//1.0,
//       								VolumetricOcclusionFactor,
//									computeKernel.data[5].z,
//									computeKernel.data[5].x,
//									computeKernel.data[5].y,
//									computeKernel.data[5].w
//       							);
//	//
//	//	float oitWeight = weightFunction(pointInCamera.z);
//	//	oitWeight *= (1.0 + pow( (height / gridHeight)*100,4.0 ) );
//	//	float newAlpha = kernelContrib.a * oitWeight;
//	//kernelContrib = addPhong(worldPosition, worldNormal,vec4(0.25,0.25,0.5,1.0), vec4(0.25,0.25,0.5,1.0),vec4(1.0),vec4(0.3,0.3,0.4,64));
//	kernelContrib.w = 1.0;
//		
//	if(kernelContrib.a < 0.01) discard;
//	//kernelContrib = vec4(sommeNormal,1.0);
//	//Color = vec4(sommeAlphaS);
//	Color = kernelContrib;//vec4(kernelColor.rgb * newAlpha , newAlpha);// * kernelColor.w;
//	
//}

