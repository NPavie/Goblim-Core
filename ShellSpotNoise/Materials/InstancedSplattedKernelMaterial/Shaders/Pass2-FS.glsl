#version 430
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Common"
#include "/Materials/Common/Kernels"						
#line 6

// constantes
#ifndef PI 
	#define PI 3.14159265359
	#define PI_2 1.57079632679
	#define PI_4 0.78539816339
#endif


layout(std140) uniform CPU
{
	mat4 objectToScreen; 	// MVP
	mat4 objectToCamera; 	// MV
	mat4 objectToWorld;		// M
	vec2 cameraPlanes;
};
uniform sampler2DArray smp_models;

uniform sampler2D smp_depthBuffer;


in vec4 v_position;
in vec3 v_normal;
in vec3 v_textureCoords;
in vec3 surface_position;
in vec3 surface_normale;
in mat4 kernelToObjectSpace;
flat in int kernelTextureID;
in kernelProfile computeKernel;

in float gridHeight;

flat in int v_invalidKernel;

in vec4 kernelBaseColor;

layout (location = 0) out vec4 Color;


void main()
{
	if(v_invalidKernel > 0.1) discard;

	float zBufferDepth = textureLod(smp_depthBuffer,gl_FragCoord.xy / vec2(1920.0,1080.0),0).x;
	//zBufferDepth = LinearizeDepth(zBufferDepth,0.01,200.0);


	bool isValid = false;
	float depthValue = 0.0;
	float height = 0.0;
	vec4 kernelContrib = getKernelColor(
			computeKernel,
			vec3(v_textureCoords.xy,0.0),
			v_textureCoords + dFdx(v_textureCoords),
			v_textureCoords + dFdy(v_textureCoords),
			smp_models,
			kernelTextureID,
			kernelToObjectSpace,
			objectToWorld,
			objectToCamera,
			vec2(0.01,200.0),
			zBufferDepth,
			surface_position,
			surface_normale,
			gridHeight,
			depthValue,
			height,
			isValid );

	if(!isValid) discard;


	float oitWeight = weightFunction(depthValue) * shellWeightFunction(height / gridHeight);
	float newAlpha = kernelContrib.a * oitWeight;
	Color = vec4(kernelContrib.rgb * newAlpha , newAlpha);// * kernelColor.w;


}

// Passe d'accumulation pondéré par OIT
//void main_old()
//{
//	if(v_invalidKernel > 0.1) discard;
//	
//	// Pour le splatting via rasterization : 
//	// les coordonnées de textures du fragment corresponde à la déprojection du fragment dans l'espace du noyau
//	vec3 kernelFragmentPosition = v_textureCoords;
//	vec3 kernelFragmentRight = v_textureCoords + dFdx(kernelFragmentPosition);
//	vec3 kernelFragmentUp = v_textureCoords + dFdy(kernelFragmentPosition);
//	kernelFragmentPosition.z = 0.0;
//	
//
//	// sample dans l'espace camera pour calcul de la profondeur
//	vec3 object_point = (kernelToObjectSpace * vec4(kernelFragmentPosition,1.0)).xyz; 
//	vec4 pointInCamera = objectToCamera * vec4( object_point , 1.0);
//	pointInCamera.z = abs(pointInCamera.z);
//
//	// Depth test du fragment :
//	float depthTest = pointInCamera.z;
//	vec4 z_b = textureLod(smp_depthBuffer,gl_FragCoord.xy / vec2(1280.0,720.0),0);
//	if(z_b.w > 0.0f && depthTest > z_b.x) discard;
//	
//	
//	vec3 VV = (object_point.xyz - surface_position.xyz);
//	float height = dot(normalize(surface_normale.xyz),VV);
//	
//	// Jacobienne du noyau dans l'espace écran
//	vec3 dx = vec3(kernelFragmentRight.xy - kernelFragmentPosition.xy,0.0);
//	vec3 dy = vec3(kernelFragmentUp.xy - kernelFragmentPosition.xy,0.0);
//	vec3 dz = vec3(0.0,0.0,0.0);
//
//	float g_vrk = getKernelContribution(
//		computeKernel, 
//		kernelFragmentPosition,
//		dx, dy, dz);
//
//	// adaptation du calcul pour choix de resultat
//	vec2 kernelTextureCoordinates = ((kernelFragmentPosition.xy) + vec2(0.5,0)).yx;
//	//kernelTextureCoordinates.y -= 0.5;
//	vec2 kernelTextureDx = vec2(0.0);
//	vec2 kernelTextureDy = vec2(0.0);
//		
//	kernelTextureDx = ((kernelFragmentRight.xy) + vec2(0.5,0)).yx;
//	kernelTextureDy = ((kernelFragmentUp.xy) + vec2(0.5,0)).yx;
//	kernelTextureDx = kernelTextureDx - kernelTextureCoordinates;
//	kernelTextureDy = kernelTextureDy - kernelTextureCoordinates;
//	
//	vec3 objectPosition = (kernelToObjectSpace * vec4(kernelFragmentPosition,1.0)).xyz;
//	vec3 objectNormal = kernelToObjectSpace[2].xyz;
//
//	vec3 worldPosition = ( objectToWorld * vec4(objectPosition,1.0)).xyz;
//	vec3 worldNormal = (objectToWorld * vec4(objectNormal,0.0)).xyz;
//	vec3 worldGroundNormal =  (objectToWorld * vec4(surface_normale.xyz,0.0)).xyz;
//
//	bool isAValidSample = kernelFragmentPosition.y > 0  
//								&&  g_vrk > 0.01 ;
//
//	bool isInTextureSpace =	all( 
//								bvec2( 
//									all(greaterThan(kernelTextureCoordinates,vec2(0.01))) , 
//									all(lessThan(kernelTextureCoordinates,vec2(0.99)))
//									) 
//								)
//							;
//
//
//	if( !isAValidSample || !isInTextureSpace) discard;
//
//
//	//vec4 kernelColor = 	textureGrad(smp_models,vec3(kernelTextureCoordinates, kernelTextureID ), kernelTextureDx, kernelTextureDy ) ;
//	
//	// -- texturing simple :
//	//bool isInTextureSpace = all( bvec2( all(greaterThan(kernelTextureCoordinates,vec2(0.01))) , all(lessThan(kernelTextureCoordinates,vec2(0.99)))) );
//	////kernelColor = g_vrk * vec4(0.05,0.5,0,1);
//	vec4 kernelColor = isInTextureSpace ? 
//		textureGrad(smp_models,vec3(kernelTextureCoordinates, kernelTextureID ), kernelTextureDx, kernelTextureDy ) : 
//		vec4(0.0);
//	kernelColor = vec4(kernelColor.rgb,kernelColor.a );	// * g_vrk
//		
//	vec4 kernelContrib;
//	float VolumetricOcclusionFactor = height / (gridHeight * (computeKernel.data[6].w));
//	float bendingTest = abs(dot(normalize(worldGroundNormal),normalize(worldNormal)));
//	
//	//if(bendingTest > 0.5) VolumetricOcclusionFactor = 0.5;
//	
//
//	//kernelContrib = g_vrk * vec4(0.05,0.5,0,1);;
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
//	
//	//kernelContrib = vec4(kernelTextureCoordinates.xy,0,g_vrk);
//	//kernelContrib = vec4(kernelBaseColor.rgb,g_vrk);
//	//kernelContrib = kernelColor;
//
//	float oitWeight = weightFunction(pointInCamera.z);
//
//	oitWeight *= (1.0 + clamp(pow( (height / gridHeight)*10,4.0 ), 0.0, 100.0) );
//	//oitWeight *= (1.0 + (pow( (height / gridHeight)*100,4.0 ) ) );
//
//	float newAlpha = kernelContrib.a * oitWeight;
//	
//	
//	if(kernelContrib.a < 0.01) discard;
//	//Color = kernelContrib * kernelContrib.a;
//	//Color = kernelContrib * newAlpha;
//	//Color = vec4(kernelContrib.rgb * newAlpha , newAlpha);// * kernelColor.w;
//	Color = vec4(kernelContrib.rgb * newAlpha , newAlpha);// * kernelColor.w;
//	
//}

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