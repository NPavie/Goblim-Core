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
	float distanceEval = XtVY(position,_Vrk,position);
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
	float distanceEval = XtVY(position,_Vrk,position);
	float g_vrk =  (1.0 / (2.0 * PI * dVrk)) * exp(-0.5 * distanceEval);
	g_vrk = clamp( config.data[1].w * g_vrk,0.0, 1.0);


	return g_vrk;
}


in vec4 v_position;
in vec3 v_normal;
in vec3 v_textureCoords;
in vec3 v_CameraSpaceCoord;
in vec3 surface_position;
in vec3 surface_normale;
in mat4 kernelToObjectSpace;
flat in int kernelTextureID;
in kernel computeKernel;

in float gridHeight;

flat in int v_invalidKernel;

in vec4 kernelBaseColor;

in mat4 cameraToObjectSpace;

layout (location = 0) out vec4 Color;


void main()
{
	vec2 screenTexCoord = gl_FragCoord.xy / vec2(1920.0,1080.0);
	vec2 screenTexCoordRight = dFdx(screenTexCoord);
	vec2 screenTexCoordUp = dFdy(screenTexCoord);

	float zBufferDepth = textureLod(smp_depthBuffer,screenTexCoord,0).x;
	if(-v_CameraSpaceCoord.z > zBufferDepth || v_invalidKernel > 0.1 ) discard;

	mat4 objectToKernelSpace = inverse(kernelToObjectSpace);	
	mat4 cameraToObjectSpace = inverse(objectToCamera);

	// Slicing v2
	// Position of the camera for the kernel
	vec3 cameraPositionInKernelSpace = (objectToKernelSpace * vec4(cameraToObjectSpace[3].xyz,1.0)).xyz;
	// Direction to the camera for the kernel
	vec3 viewingDirectionInKernelSpace = -normalize(cameraPositionInKernelSpace);
	// Direction to the camera for slicing
	vec3 viewingDirectionInObjectSpace = -normalize(cameraToObjectSpace[3].xyz - kernelToObjectSpace[3].xyz);
	
	float sommeAlphaS = 0.0;
	vec4 kernelColor = vec4(0.4,0.8,0.3,1.0);
	vec4 sommeColor = vec4(0.0);

	// Direction kernel->camera normalized dans l'espace noyau puis retransformer dans l'espace objet
	vec3 kernelFragmentPosition;
	
	int nbSlice = 10;
	float depthValue = 0.0;

	float slicingStep = length( (kernelToObjectSpace * vec4(viewingDirectionInKernelSpace,0.0) ).xyz ) / nbSlice;
	vec3 firstSlice = kernelToObjectSpace[3].xyz - viewingDirectionInObjectSpace * (nbSlice * 0.5) * slicingStep;

	for(int sl = 0; sl < nbSlice; ++sl)
	{
		
		vec3 currentSliceCenter = firstSlice + viewingDirectionInObjectSpace * sl * slicingStep  ;
		
		// Pour évaluer le point de la tranche, on projette l'espace de la tranche dans l'espace écran
		mat3 pixelToSliceSpace = project2DSpaceToScreen(currentSliceCenter ,cameraToObjectSpace[0].xyz , cameraToObjectSpace[1].xyz, objectToScreen);
		// L'espace obtenu permet de transformer un point de l'espace de la slice vers l'espace écran, du coup on l'inverse
		pixelToSliceSpace = inverse(pixelToSliceSpace);

		// Pour recalculer la position correspondante dans l'espace objet, on transforme le pixel en espace de la tranche
		vec3 pixelPositionInSlice = pixelToSliceSpace * vec3(screenTexCoord.xy,1.0);
		// Et on réutilise ces coordonnées pour recaluler la position dans l'objet à partir de des axes de la slice
		pixelPositionInSlice = currentSliceCenter + pixelPositionInSlice.x * cameraToObjectSpace[0].xyz + pixelPositionInSlice.y * cameraToObjectSpace[1].xyz;
		
		// Echantillon du noyau sélectionné
		kernelFragmentPosition = (objectToKernelSpace * vec4(pixelPositionInSlice,1.0)).xyz;

		// information pour calcul dans la surcouche
		vec3 pointInObject = pixelPositionInSlice;
		vec3 VV = (pointInObject.xyz - surface_position.xyz);
		float height = dot(normalize(surface_normale.xyz),VV);

		// sample dans l'espace camera pour calcul de la profondeur 
		vec4 pointInCamera = objectToCamera * vec4( pointInObject , 1.0);
		depthValue = -pointInCamera.z;
		if(-pointInCamera.z > zBufferDepth) break;
		
		vec3 normal = vec3(0.0);
		float g_vrk = computeKernelBase(
						vec3(0.0), 
    					computeKernel.data[1],
    					kernelFragmentPosition,
    					normal);
		//computeStars(vec3(0.0), vec3(1.0) ,kernelFragmentPosition);

		vec3 worldPosition = (objectToWorld * vec4(pointInObject,1.0)).xyz;
		vec3 worldNormal = normalize( (objectToWorld * kernelToObjectSpace * vec4(normalize(normal),0.0)).xyz );

		vec3 worldGroundNormal = normalize( (objectToWorld * vec4(surface_normale,1.0)).xyz );
		float VolumetricOcclusionFactor = height / (gridHeight * (computeKernel.data[6].w));
		
		vec4 kernelContrib = addBoulanger3(	
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
		
		//vec4 kernelContrib = kernelColor;

		//sommeAlphaS += g_vrk;
		sommeColor += vec4(kernelContrib.rgb * g_vrk, g_vrk);
	}

	if(sommeColor.a > 1.0) sommeColor.rgb /= sommeColor.a;
	else discard;

	if(sommeColor.a > 1.0) sommeColor.a = 1.0;


	// Splatting with OIT :
	float oitWeight = weightFunction(depthValue);// * shellWeightFunction(height / gridHeight);
	float newAlpha = sommeColor.a * oitWeight;
	Color = vec4(sommeColor.rgb * newAlpha , newAlpha);// * kernelColor.w;
	
}

