#version 430
#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Kernels"						
#line 5

layout (location = 0) in vec3 Position;
layout (location = 3) in vec3 Texture_Coordinates;

out gl_PerVertex
{
  vec4 gl_Position;
  float gl_PointSize;
  float gl_ClipDistance[];
};

// UNIFORMS
layout(std140) uniform CPU
{
	mat4 objectToWorldSpace;
	float f_Grid_height;
};

uniform sampler2DArray smp_densityMaps;
uniform sampler2DArray smp_scaleMaps;
uniform sampler2DArray smp_distributionMaps;
uniform	sampler2DArray smp_colorMaps;
uniform sampler2DArray smp_surfaceData;
// ------------

// OUTPUT
flat out vertexGeneratedImpulse surfaceKernel;
out vec4 kernelEvaluationPoint;
out float gridHeight;
// ------------

void main()
{

	int profileID = 0;
	int instanceID = gl_InstanceID;
	int kernelCount = 0;
	gridHeight = f_Grid_height;


	for(int i = 0; i < nbModels.x; i++)
	{
		int kernelCountForI = int(Kernels[i].data[2].y * Kernels[i].data[2].y * Kernels[i].data[2].z);
		if(instanceID < kernelCountForI)
		{
			profileID = i;
			break;
		}
		else instanceID -= kernelCountForI;		
	}

	kernelProfile currentProfile = Kernels[profileID];

	// index of kernel and cell
	int nbCellPerLine = int(currentProfile.data[2].y);
	int kernelPerCell = int(currentProfile.data[2].z);
	int numKernel = instanceID % kernelPerCell;
	int Cell1DId = instanceID / kernelPerCell;

	ivec2 instanceCell;
	instanceCell.x = Cell1DId % nbCellPerLine;
	instanceCell.y = Cell1DId / nbCellPerLine;



	surfaceKernel = getNewKernelSpace(
						profileID,											// ID in the Kernels[] array from the SSBO
						vec4(vec2(instanceCell),vec2(nbCellPerLine)), 		// cellInfo.xy = distributionCell.xy, cellInfo.zw = nbCellsPerAxis
						numKernel,											// number of the kernel in the distribution cell
						gridHeight,
						smp_surfaceData,									// 3D position, normal, tangent
						smp_densityMaps, 									// kernel parameters : density mapped over the surface
						smp_scaleMaps, 										// kernel parameters : scale factor mapped over the surface
						smp_distributionMaps, 								// kernel parameters : normal distribution mapped over the surface (kernels direction
						smp_colorMaps
						);
	
	// Evaluation pour le quad
	kernelEvaluationPoint = vec4((Texture_Coordinates - vec3(0.5)) * 2.0f, 1.0); // Pour mettre les UV entre -1 et 1 (comme les vertex du quad)

	// Rasterization
	gl_Position = (ViewProj * objectToWorldSpace * surfaceKernel.toObjectSpace * vec4(Position,1.0));

}
