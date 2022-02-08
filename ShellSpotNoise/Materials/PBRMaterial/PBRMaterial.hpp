#ifndef PBR_MATERIAL_HPP_
#define PBR_MATERIAL_HPP_
#include "Engine/OpenGL/MaterialGL.h"
#include "Engine/OpenGL/Lighting/LightingModelGL.h"
#include "GPUResources/Textures/GPUTextureCubeMap.h"
#include <memory.h>

class PBRMaterial : public MaterialGL {
public:
	PBRMaterial (std::string name, GPUTextureCubeMap* cube = NULL);
	PBRMaterial (std::string name, glm::vec3 base, float smoothness, float metal, float reflectance);
		~PBRMaterial ();
		void setTime (float t);

		virtual void render (Node *o);
		virtual void update (Node* o, const int elapsedTime);
		GPUmat4* modelViewProj;
		GPUvec4* color;
		GPUmat4* M;
		GPUmat4* normalM;

		// --- Tests pour éclairage ambiant avec PBR
		GPUsampler* fp_cubeMap; 
		GPUTextureCubeMap* cubeMap;
		GPUbool* fp_hasCubeMap;
		bool hasCubeMap=false;
		// ---

		GPUfloat* roughness;
		GPUvec3 *F0;
		GPUvec3 *albedo;

		GPUvec3* camPos;

		// ---- Pour interface AntTweakBar dans l'Engine
		float metalMask;
		float reflectance;
		float smoothness;

		void updateValues (float smoothness, float metalMask, float reflectance, glm::vec3 baseColor) {
			this->smoothness = smoothness;
			this->metalMask = metalMask;
			this->reflectance = reflectance;

			roughness->Set (1.0f - smoothness*smoothness);
			F0->Set (0.16f*(reflectance*reflectance)*(1.0f - metalMask) + baseColor*metalMask);
			albedo->Set (baseColor);
		}

		void setRoughness (float s);
		void setF0 (glm::vec3 f);
		void setAlbedo (glm::vec3 a);

		float getSmoothness () {
			return glm::sqrt(1.0f - roughness->getValue ());
		}
		glm::vec3 getF0 () {
			return F0->getValue ();
		}
		glm::vec3 getAlbedo () {
			return albedo->getValue ();
		}
		float getReflectance () {
			return reflectance;
		}
		float getMetalMask () {
			return metalMask;
		}
		// ---- 
	};

#endif