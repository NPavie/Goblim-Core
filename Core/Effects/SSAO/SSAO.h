#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>
#include "Engine/OpenGL/ModelGL.h"
#include "GPUResources/Textures/GPUTexture2D.h"
#include "Engine/Base/Scene.h"
#include "Engine/OpenGL/LightingModelGL.h"
#include "Materials/NormalMaterial/NormalMaterial.h"
#include "Engine/Base/NodeCollector.h"
#include "Effects/BilateralFilter/BilateralFilter.h"

class SSAO : public EffectGL
{
	public:
		SSAO(std::string name,int size = 512);
		~SSAO();

		virtual void computeSSAO(NodeCollector* collector);
		virtual void apply();
	
		void setOccluderBias(float f);
		void setSamplingRadius(float f);
		void setAttenuation(glm::vec2 f);
	protected:
		int fboSize;
		
		GPUsampler *normalMapSampler,*randomMapSampler;
		GLProgram *ssaoShader;
		GLProgram *vp;
		NormalMaterial *normalMat;
		Scene *scene;
		GPUTexture2D* noise;
		GPUvec2 *texelSize;
		GPUfloat *occluderBias,*samplingRadius;
		GPUvec2 *attenuation;
		GPUFBO* normalFBO,*ssaoFBO;

		BilateralFilter* blur;
};
