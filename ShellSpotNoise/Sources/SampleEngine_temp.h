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
#include "Materials/TextureMaterial/TextureMaterial.h"
#include "Materials/ColorMaterial/ColorMaterial.h"
#include "Materials/DepthMaterial/DepthMaterial.h"

//#include "Materials/VolumetricKernelMaterial/VolumetricKernelMaterial.h"
#include "Effects/GrassComposer/GrassComposer.h"
#include "Materials/DepthMaterial/DepthMaterial.h"

#include "Materials/InstancedSplattedKernelMaterial/InstancedSplattedKernelMaterial.h"
#include "Materials/InstancedKernelMaterial/InstancedKernelMaterial.h"
#include "Materials/DepthPeeledShellMaterial/DepthPeeledShellMaterial.h"
#include "Materials/VolumetricShellKernelMaterial/VolumetricShellKernelMaterial.h"
#include "Materials/InstancedVolumetricKernelMaterial/InstancedVolumetricKernelMaterial.h"

#include "Effects/BendGrass/BendGrass.h"

#include "KernelList.h"


class SampleEngine : public EngineGL
{
	public:
		SampleEngine(int width, int height);
		~SampleEngine();

		virtual bool init();
		virtual void render();
		virtual void animate(const int elapsedTime);
		virtual void onWindowResize(int w, int h);

		bool activeCapture;
		GrassComposer*						grassPostProcess;
		InstancedSplattedKernelMaterial*	grassSplatted;
		InstancedKernelMaterial*			grassInstanced;
		DepthPeeledShellMaterial*			grassPeeled;
		InstancedVolumetricKernelMaterial*		grassVolumic;

		void useInstancing();
		void useSplatting(bool useBestSample);
		void useComposer();

		void changeSplatting();
		void changeBestFragment();


		int frameCounter;

	protected:
		
		GPUFBO* screenSizeFBO;
		GPUFBO* mainRenderer;
		GPUFBO* depthRenderer;

		TextureMaterial* solTexture;

		KernelList* kernelsConfiguration;

		DepthMaterial* depthCompute;
		//VolumetricKernelMaterial* voxelBaseRendering;

		BendGrass* bendGrass;
		
		Node* ground, *pierre; // kiroulenamassepasmousse
		
};
#endif
