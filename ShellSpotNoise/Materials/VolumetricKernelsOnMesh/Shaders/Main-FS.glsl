#version 430
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Kernels"
#line 5

layout(std140) uniform CPU
{
	mat4 objectToWorld;		// M
	int volKernelID;
};
uniform sampler2DArray smp_models;
uniform sampler2D smp_depthBuffer;


layout (location = 0) out vec4 Color;


// INPUT
in vec4 kernelEvaluationPoint;
flat in vertexGeneratedImpulse surfaceKernel;
in vec3 slicingDirectionInKernel;
in float gridHeight;
// ------------

const int nbSlice = 32;
const float steping = 1.0 / nbSlice;

void main()
{

	kernelProfile currentProfile = Kernels[surfaceKernel.profileID];

	if( surfaceKernel.validationInt > 0.1 ) discard;
	
	vec2 screenTexCoord = gl_FragCoord.xy / vec2(1920.0,1080.0);
	float zBufferDepth = textureLod(smp_depthBuffer,screenTexCoord,0).x;
	if(abs(gl_FragCoord.z) > zBufferDepth) discard;

	vec4 kernelColor = vec4(0.2,0.3,0.5,1.0);
	//kernelColor = vec4(0.1,0.6,0.3,1.0);
	//kernelColor = vec4(0.7,0.1,0.2,1.0);
	//kernelColor = currentProfile.data[9];
	vec4 colorInProfile = currentProfile.data[9];
	kernelColor = surfaceKernel.colorFromMap;

	kernelColor.rgb = currentProfile.data[4].w * kernelColor.rgb + (1.0 - currentProfile.data[4].w) * colorInProfile.rgb;

	mat4 surfaceKernel_toWorldSpace = objectToWorld * surfaceKernel.toObjectSpace;
	mat4 surfaceKernel_toViewSpace = View * surfaceKernel_toWorldSpace;


	mat4 viewSpaceToKernel = inverse(surfaceKernel_toViewSpace);


	//vec3 dirSlicing = normalize(slicingDirectionInKernel);
	vec3 dirSlicing = normalize(-viewSpaceToKernel[3].xyz);

	vec3 firstSliceCenterInKernel = -0.5 * nbSlice * steping * dirSlicing;
	
	vec3 firstPosition = kernelEvaluationPoint.xyz + dirSlicing * dot( firstSliceCenterInKernel - kernelEvaluationPoint.xyz, dirSlicing);

	mat4 surfaceKernel_toScreen = Proj * surfaceKernel_toViewSpace;

	vec3 positionFinale = vec3(0.0);
	vec3 normaleFinale = vec3(0.0);
	vec4 colorAccumulation = vec4(0.0);
	float profondeurFinale = 0.0;
	float accumulateur = 0.0;


	// TODO : ne garder que la première slice contributive ?

	for(int sl = 0; sl < nbSlice && accumulateur < 1.0; sl++)
	{

		vec3 positionInKernelSpace = firstPosition + sl * steping * dirSlicing;

		// Depth test
		vec4 pointInScreen = ViewProj * surfaceKernel_toWorldSpace * vec4(positionInKernelSpace,1.0);
		pointInScreen.xyz /= pointInScreen.w;
		pointInScreen.xyz = vec3(0.5) + pointInScreen.xyz * vec3(0.5);

		if(abs(pointInScreen.z) > zBufferDepth) break;

		//vec3 normal = vec3(0.0);
		//vec4 test = donut(
		//			//champignon(
		//			vec3(0.0), 
		//			currentProfile.data[1].xyz * currentProfile.data[1].w,
		//			positionInKernelSpace);

		vec4 test = selectedVolumetricKernel(
			vec3(0.0), 
			currentProfile.data[1].xyz * currentProfile.data[1].w,
			positionInKernelSpace,
			volKernelID
			);


		positionFinale += (positionInKernelSpace * test.w);
		colorAccumulation += kernelColor * test.w;
		accumulateur += test.w;

		profondeurFinale += pointInScreen.z * test.w;


	}
	if(accumulateur < 0.01) discard;

	float accumulateur_restant = 1.0 - accumulateur;
	
	positionFinale /= accumulateur;

	profondeurFinale /= accumulateur;
	gl_FragDepth = profondeurFinale;

	accumulateur = clamp(accumulateur,0.0,1.0);

	//normaleFinale = donut(
	//				//champignon(
	//				vec3(0.0), 
    //				currentProfile.data[1].xyz * currentProfile.data[1].w,
    //				positionFinale).xyz;

	normaleFinale = selectedVolumetricKernel(
				vec3(0.0), 
				currentProfile.data[1].xyz * currentProfile.data[1].w,
				positionFinale,
				volKernelID
				).xyz;

	vec3 pointInObject = (surfaceKernel.toObjectSpace * vec4(positionFinale,1.0)).xyz;		
	vec3 VV = (pointInObject.xyz - surfaceKernel.relatedSurfacePoint.position.xyz);
	float height = dot(normalize(surfaceKernel.relatedSurfacePoint.normale.xyz),VV);

	vec3 worldPosition = (surfaceKernel_toWorldSpace * vec4(positionFinale,1.0)).xyz;

	vec3 worldNormal = ( (surfaceKernel_toWorldSpace * vec4( positionFinale + normaleFinale,1.0)).xyz );
	worldNormal = normalize(worldNormal - worldPosition);

	vec3 worldGroundNormal = normalize( (objectToWorld * vec4(surfaceKernel.relatedSurfacePoint.normale.xyz,0.0)).xyz );
	float VolumetricOcclusionFactor = height / (gridHeight * (currentProfile.data[6].w));
	
	colorAccumulation = addPhong(worldPosition,worldNormal, kernelColor, kernelColor ,vec4(1.0),vec4(0.1,0.8,0.3,128.0));

	//colorAccumulation += kernelColor * accumulateur_restant;

	//colorAccumulation = addBoulanger3(	
	//							worldPosition,
   	//							kernelColor,
   	//							worldNormal,
   	//							worldGroundNormal,
   	//							//1.0,
   	//							VolumetricOcclusionFactor,
	//							currentProfile.data[5].z,
	//							currentProfile.data[5].x,
	//							currentProfile.data[5].y,
	//							currentProfile.data[5].w
   	//						);
	


	//colorAccumulation /= accumulateur;

	Color = vec4(colorAccumulation.xyz * colorInProfile.a,accumulateur * colorInProfile.a);
	
}

/*

	// Pour le slicing, on utilise l'espace camera pour avoir des slice perpendiculaire à la direction de la camera
	//vec3 slicingDirectionInKernel = normalize( (viewSpaceToKernel * vec4(0,0,1,0)).xyz);
	vec4 cameraCenterInKernel 	= (viewSpaceToKernel * vec4(0,0,0,1));//.xyz;
	vec4 cameraXInKernel 		= (viewSpaceToKernel * vec4(1,0,0,1));//.xyz;
	vec4 cameraYInKernel 		= (viewSpaceToKernel * vec4(0,1,0,1));//.xyz;
	vec4 cameraZInKernel 		= (viewSpaceToKernel * vec4(0,0,1,1));//.xyz;

	cameraCenterInKernel = cameraCenterInKernel / cameraCenterInKernel.w ;
	cameraXInKernel	= cameraXInKernel / cameraXInKernel.w; 	 
	cameraYInKernel	= cameraYInKernel / cameraYInKernel.w; 	 
	cameraZInKernel	= cameraZInKernel / cameraZInKernel.w; 	 

	vec3 slicingDirectionInKernel = normalize( cameraZInKernel.xyz - cameraCenterInKernel.xyz);
	vec3 sliceUpVectorInKernel = 	normalize( cameraYInKernel.xyz - cameraCenterInKernel.xyz);
	vec3 sliceRightVectorInKernel = normalize( cameraXInKernel.xyz - cameraCenterInKernel.xyz);


	//vec3 slicingDirectionInKernel = normalize( (viewSpaceToKernel * vec4(0,0,1,0)).xyz);
	//vec3 sliceUpVectorInKernel = 	normalize( (viewSpaceToKernel * vec4(0,1,0,0)).xyz);
	//vec3 sliceRightVectorInKernel = normalize( (viewSpaceToKernel * vec4(1,0,0,0)).xyz);
	// Pour la version "perspective aligned", prendre la direction camera->voxel et reconstruire les axes up et right 


	vec3 firstSliceCenterInKernel = -(0.5 * nbSlice * steping * slicingDirectionInKernel);
	mat4 surfaceKernel_toScreen = Proj * surfaceKernel_toViewSpace;

	// Matter accumulation
	vec4 colorAccumulation = vec4(0.0);

	float meanDepth = 0.0;

	for(int sl = 0; sl < nbSlice && colorAccumulation.w < 1.0; sl++)
	{
		vec3 currentSliceCenter = firstSliceCenterInKernel + sl * steping * slicingDirectionInKernel;
		mat3 sliceToPixelSpace = project2DSpaceToScreen(currentSliceCenter, sliceRightVectorInKernel , sliceUpVectorInKernel, surfaceKernel_toScreen);
		mat3 pixelToSliceSpace = inverse(sliceToPixelSpace);

		vec3 pixelInSliceSpace = pixelToSliceSpace * vec3(screenTexCoord,1.0);
		vec3 positionInKernelSpace = currentSliceCenter + sliceRightVectorInKernel * pixelInSliceSpace.x + sliceUpVectorInKernel * pixelInSliceSpace.y;

		//vec3 normal = vec3(0.0);
		float g_vrk = JMD_2(
						vec3(0.0), 
    					currentProfile.data[1].xyz * currentProfile.data[1].w,
    					positionInKernelSpace);

		vec3 pointInObject = (surfaceKernel.toObjectSpace * vec4(positionInKernelSpace,1.0)).xyz;
		vec3 VV = (pointInObject.xyz - surfaceKernel.relatedSurfacePoint.position.xyz);
		float height = dot(normalize(surfaceKernel.relatedSurfacePoint.normale.xyz),VV);

		vec3 worldPosition = (surfaceKernel_toWorldSpace * vec4(positionInKernelSpace,1.0)).xyz;
		//vec3 worldNormal = normalize( (surfaceKernel_toWorldSpace * vec4(normalize(normal),0.0)).xyz );
		vec3 worldNormal = normalize( (objectToWorld * vec4(surfaceKernel.relatedSurfacePoint.normale.xyz,0.0)).xyz );

		vec3 worldGroundNormal = normalize( (objectToWorld * vec4(surfaceKernel.relatedSurfacePoint.normale.xyz,0.0)).xyz );
		float VolumetricOcclusionFactor = height / (gridHeight * (currentProfile.data[6].w));
		
		vec4 kernelContrib = addBoulanger3(	
									worldPosition,
	   								kernelColor,
	   								worldNormal,
	   								worldGroundNormal,
	   								//1.0,
	   								VolumetricOcclusionFactor,
									currentProfile.data[5].z,
									currentProfile.data[5].x,
									currentProfile.data[5].y,
									currentProfile.data[5].w
	   							);
		

		colorAccumulation += vec4(kernelContrib.rgb * g_vrk, g_vrk);

		vec4 pointInScreen = Proj * View * vec4(worldPosition,1.0);
		pointInScreen.xyz /= pointInScreen.w;
		pointInScreen.xyz = vec3(0.5) + pointInScreen.xyz * vec3(0.5);

		meanDepth += abs(pointInScreen.z) * g_vrk;

	}

	*/