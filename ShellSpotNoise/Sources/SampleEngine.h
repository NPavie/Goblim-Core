/*
 *	(c) XLim, UMR-CNRS
 *	Authors: G.Gilet
 *
 */

#ifndef _SAMPLE_ENGINE_H
#define _SAMPLE_ENGINE_H

#include <map>
#include <string>
#include "Engine/OpenGL/EngineGL.h"
#include "Materials/BoundingBoxMaterial/BoundingBoxMaterial.h"

#include "Materials/DeferredMaterial/DeferredMaterial.h"
#include "Effects/PostProcess/PostProcess.h"
#include "Effects/GrainEffect/GrainEffect.h"
#include "Materials/PBRMaterial/PBRMaterial.hpp"
#include "Materials/ColorMaterial/ColorMaterial.h"
#include "Materials/TextureMaterial/TextureMaterial.h"

#include "Materials/DepthMaterial/DepthMaterial.h"
#include "Materials/ImplicitSurfaceMaterial/ImplicitSurfaceMaterial.h"
#include "Materials/VolumetricShellKernelMaterial/VolumetricShellKernelMaterial.h"
#include "Materials/Deferred3DKernelMaterial/Deferred3DKernelMaterial.h"
#include "Effects/BendGrass/BendGrass.h"

#include "Materials/FlatKernelsOnMesh/FlatKernelsOnMesh.h"
#include "Materials/VolumetricKernelsOnMesh/VolumetricKernelsOnMesh.h"

#include "KernelList.h"

class SampleEngine : public EngineGL
{
	public:
		SampleEngine(int width, int height);
		~SampleEngine();

		virtual bool init();
		virtual void render();
		virtual void animate(const int elapsedTime);
		virtual void onWindowResize(int width, int height);

		float smoothness;// = 0.86;
		glm::vec3 F0;
		glm::vec3 albedo;
		glm::vec3 lightPos;


		// Recharger le PostProcess
		void reloadPostProcess ();

		FlatKernelsOnMesh*					flatKernelsRendering;
		VolumetricKernelsOnMesh*			volumetricKernelsRendering;
		
		VolumetricShellKernelMaterial*		voxelizationTest;

		ImplicitSurfaceMaterial*			implicitSurfaceKernel;
		Deferred3DKernelMaterial*			kernel3DTest;
		
		GPUFBO* GeometryPassFBO;
		GPUFBO* PostProcessFBO;
		DeferredMaterial *deferredMat;


		ColorMaterial* defaultMaterial;
		TextureMaterial* groundDefault;

		GPUFBO *FBO_Grain;
		GrainEffect* grainEffect;

		// Textures
		GPUTexture2D* texDamier;
		GPUTexture2D* texDamierGold;
		GPUTexture2D* normalDamier;
		GPUTexture2D* roughnessDamier;
		GPUTexture2D* texWood;
		GPUTexture2D* normalWood;
		GPUTexture2D* dirt;

		void useSplatting(bool useBestSample);

	protected:

		GPUFBO* screenSizeFBO;
		GPUFBO* mainFrameBuffer;
		GPUFBO* depthRenderer;

		//TextureMaterial* solTexture;

		KernelList* kernelsConfiguration;

		DepthMaterial* depthCompute;
		//VolumetricKernelMaterial* voxelBaseRendering;

		BendGrass* bendGrass;

		Node* ground, *pierre; // kiroulenamassepasmousse // sponza_117
		Node* shellNode;

		

		//SampleEffect* sampleEffectForTest;
		//SampleMaterial* sampleMaterialForTest;

		bool useMB = true;
		
		
		

};
#endif
