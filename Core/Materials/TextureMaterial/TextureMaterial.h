#ifndef _TEXTURE_MATERIAL_H
#define _TEXTURE_MATERIAL_H

#include "Engine/OpenGL/MaterialGL.h"
#include "GPUResources/Textures/GPUTexture2D.h"
#include "Engine/OpenGL/Lighting/LightingModelGL.h"
#include <memory.h>

class TextureMaterial : public MaterialGL
{
	public:
		TextureMaterial(std::string name, GPUTexture *color,GPUTexture *normal = NULL,LightingModelGL *lightingM = NULL);
		~TextureMaterial();

		virtual void render(Node *o);
		virtual void update(Node* o,const int elapsedTime);

		
		void setNormalMap(GPUTexture *normal);
		void setColorMap(GPUTexture *color);
		void useNormal(bool b);

		GPUmat4 *modelViewProj,*modelView,*modelViewF, *modelMatrix;

		GPUsampler *colorSampler,*normalSampler;
		GPUTexture *colorTex,*normalTex;
		GPUbool *use_normal;
		GPUsampler* test;
		GPUfloat* valGpu;

		void changeRes();
		
	protected:
		bool loadShaders();
		LightingModelGL *lighting;
		int val;
};
#endif