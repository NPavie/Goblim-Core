#include "Engine/OpenGL/MaterialGL.h"
#include "GPUResources/Textures/GPUTexture2D.h"
#include "Engine/OpenGL/Lighting/LightingModelGL.h"
#include <memory.h>

#ifndef DEFERREDMATERIAL_H
#define DEFERREDATERIAL_H

struct Texture_Sampler {
	Texture_Sampler (GPUTexture* t, GPUsampler* s) {
		tex = t;
		sampler = s;
	}
	GPUTexture* tex;
	GPUsampler* sampler;
};

class DeferredMaterial : public MaterialGL
{
	public:
		DeferredMaterial (std::string name, 
			GPUTexture *color = NULL, 
			GPUTexture *ao = NULL, 
			GPUTexture *metalMask = NULL, 
			GPUTexture *roughness = NULL, 
			GPUTexture *reflectance = NULL,  
			GPUTexture *normal = NULL, 
			float bumpScale = 0.0f, 
			float maxQuality = 0.0f, 
			bool height = false, 
			bool ss = false,
			glm::vec3 metal_reflectance_roughness=glm::vec3(0.0, 0.3, 0.87),
			glm::vec3 baseColor = glm::vec3 (0.5),
			LightingModelGL *lightingM = NULL);
		~DeferredMaterial();

		virtual void render(Node *o);
		virtual void update(Node* o, const int elapsedTime);
		
		//----- Vertex Program Parameters ---------------------//
		GPUmat4 *vs_MV, *vs_M, *vs_MVP, *vs_NormalMV, *vs_NormalM, *vs_LastMVP;
		//----- Tesselation Control Program Parameters --------//
		//----- Tesselation Evaluation Program Parameters -----//
		//----- Geometry Program Parameters -------------------//
		GPUmat4 *gs_MV, *gs_MVP, *gs_NormalMV;
		GPUvec3 *gs_CamPos;
		//----- Fragment Program Parameters -------------------//
		//GPUmat4 *fs_M, *fs_NormalMV, *fs_NormalM;
		GPUvec3 *fs_CamPos;
		GPUfloat *fs_bumpScale, *fs_maxQuality;
		GPUbool *fs_use_ao, *fs_use_metalMask, *fs_use_base_color, *fs_use_roughness, *fs_use_reflectance, *fs_use_normal, *fs_use_height, *fs_use_selfshadow;
		/*GPUTexture *fs_colorTex, *fs_aoTex, *fs_metalMaskTex, *fs_roughnessTex, *fs_reflectanceTex, *fs_normalTex;
		GPUsampler *fs_colorSampler, *fs_aoSampler, *fs_metalMaskSampler, *fs_roughnessSampler, *fs_reflectanceSampler, *fs_normalSampler;*/
		GPUvec3 *fs_baseColor, *fs_metalMask_reflectance_roughness;

	protected:
		bool loadShaders();
		std::vector<Texture_Sampler> texturesGPU;
		LightingModelGL *lighting;
};
#endif