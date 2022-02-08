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

#include "Effects/SampleEffect/SampleEffect.h"
#include "Materials/SampleMaterial/SampleMaterial.h"

class SampleEngine : public EngineGL
{
	public:
		SampleEngine(int width, int height);
		~SampleEngine();

		virtual bool init();
		virtual void render();
		virtual void animate(const int elapsedTime);

		void requestUpdate();

	protected:

		//SampleEffect* sampleEffectForTest;
		//SampleMaterial* sampleMaterialForTest;

		
		
		

};
#endif
