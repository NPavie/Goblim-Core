/*
*	Authors: G. THOIN
*	Master 2 ISICG
*/

#ifndef _POSTPROCESS_EFFECT_
#define _POSTPROCESS_EFFECT_

#include "Engine/OpenGL/EffectGL.h"
#include <memory.h>
#include "Engine/OpenGL/ModelGL.h"
#include <omp.h>
#include <time.h>
#include <chrono>
class PostProcess : public EffectGL
{
	public:
		PostProcess(std::string name, 
					bool useHDRPostProcess = false,
					bool useAA = false,
					bool useDO = false,
					float numSamples = 32.0,
					int noiseSize = 8,
					bool useTM = false,
					bool useTemporalTM = false,
					float exposure = 1.0f,
					bool useBl = false,
					float threshold = 0.8f,
					int blurSize = 128,
					bool useDOF = false,
					bool useNearDOF = false,
					bool useFarDOF = false,
					float nearFocal = 0.2f,
					float farFocal = 0.8f,
					int focalBlurSize = 9,
					int shadingBlurSize = 3,
					bool useFog = false,
					bool useMB = false,
					bool useFinal = false
					);

		~PostProcess();

		virtual void apply(GPUFBO *in, GPUFBO *out);

		void reload (float threshold,
					int blurSize,
					float nearFocal,
					float farFocal,
					int focalBlurSize,
					int shadingBlurSize,
					float numSamples,
					int noiseSize);


		void setExposure (float e);
	
		bool useDO;				// Use Directional occlusion
		float doBaseRadiusVal = 0.0017;
		float doMaxOcclusionDistVal = 1.81f;

		void setNearFocal (float near) {
			dofFocalNear->Set (near);
		}
		void setFarFocal (float far) {
			dofFocalFar->Set (far);
		}

		void setMMVelocityScale (float v) {
			velocityScale->Set (v);
		}
			
	protected:

		//----- Effect Initialization Functions -----//
		void createPrograms();
		void createFBOs();
		void createSamplers();
		void createGPUParameters (float threshold, int blurSize, float nearFocal, float farFocal, int focalBlurSize, int shadingBlurSize, float exposure);
		void computeAOPreProcess(int numSamples, int noiseSize);

		//----- Effect Applying Functions -----//
		void applyLDRPostProcess(GPUFBO *GBuffers, GPUFBO *out);
		void applyHDRPostProcess(GPUFBO *GBuffers, GPUFBO *out);
		void draw();
		void applyDepthLinear(GPUFBO *GBuffers, GPUFBO *out);
		void applyDeferredShading(GPUFBO *GBuffers, GPUFBO *AO, GPUFBO *out);
		void applyAntiAliasing(GPUFBO *GBuffers, GPUFBO *Depth_FBO_Normalized, GPUFBO *Shading, GPUFBO *out);
		void applyDirectionalOcclusion(GPUFBO *in, GPUFBO *depth, GPUFBO *out);
		void applyToneMapping(GPUFBO *HDRShading, GPUFBO *out);
		void applyBloom(GPUFBO *Shading, GPUFBO *out);
		void applyDepthOfField(GPUFBO *Depth_FBO_Normalized, GPUFBO *Shading, GPUFBO *out);
		void applyFog (GPUFBO *Depth_FBO_Normalized, GPUFBO *Shading, GPUFBO *out);
		void applyTest (GPUFBO* depth, GPUFBO *Shading, GPUFBO *out);
		void applyMotionBlur (GPUFBO *GBuffers, GPUFBO *Shading, GPUFBO *out);

		void bindNoiseTexture(GLuint channel);
		void releaseNoiseTexture();

		//----- Effect Parameters -----//
		bool useHDRPostProcess;	// Use HDR Post Processing (Tone Mapping done last !)
		bool useAA;				// Use AntiAliasing
		bool useTM;				// Use Tone Mapping
		bool useTemporalTM;		// Use Temporal Tone Mapping
		bool useBl;				// Use Bloom
		bool useDOF;			// Use Depth of Field
		bool useNearDOF;		// Use Near Depth of Field
		bool useFarDOF;			// Use Far Depth of Field
		bool useFog;			// Use Fog
		bool useMB;				// Use Motion Blur
		bool useFinal;			// Use final shader test

		//----- GLSL Programs -----//
		GLProgram *Base;							// Vertex Shader Pass (Darw a screen aligned quad)

		GLProgram *Depth_PRG_Normalize;				// Normalize Depth Buffer

		GLProgram *DS_PRG_Shading;					// Deferred Shading Pass (Shade the scene using the G-Buffers)

		GLProgram *AA_PRG_Edge_EdgeDetection;		// Edge detection Pass
		GLProgram *AA_PRG_Edge_HBlur;				// Edge horizontal blur Pass
		GLProgram *AA_PRG_Edge_VBlur;				// Edge vertical blur Pass
		GLProgram *AA_PRG_Shading_HBlur;			// Shading blur Pass
		GLProgram *AA_PRG_Shading_VBlur;			// Shading blur Pass
		GLProgram *AA_PRG_AntiAliasing;				// AntiAliasing Pass (Blur the edges in shaded image)

		GLProgram *DO_PRG_DO;						// Directional Occlusion Pass
		GLProgram *DO_PRG_DO_HBlur;					// DO horizontal blur Pass
		GLProgram *DO_PRG_DO_VBlur;					// DO vertical blur Pass

		GLProgram *TM_PRG_Luminance_Computation;	// Luminance computation Pass (Compute each pixel luminance)
		GLProgram *TM_PRG_Luminance_Downscale;		// Luminance downscale Pass (Compute image max luminance)
		GLProgram *TM_PRG_ToneMapping;				// Tone Mapping (Linearize the scene brightness)

		GLProgram *Bl_PRG_Bright_Computation;		// Bright computation Pass (Store every pixel above a threshold)
		GLProgram *Bl_PRG_Bright_HBlur;				// Bright horizontal blur Pass
		GLProgram *Bl_PRG_Bright_VBlur;				// Bright vertical blur Pass
		GLProgram *Bl_PRG_Bloom;					// Bloom Pass (Blend shaded image and blurred bright pixels)

		GLProgram *DOF_PRG_Focal_Depth;				// Focal depth Pass (Test if a pixel is in focal range)
		GLProgram *DOF_PRG_Focal_HBlur;				// Focal horizontal blur Pass
		GLProgram *DOF_PRG_Focal_VBlur;				// Focal vertical blur Pass
		GLProgram *DOF_PRG_Shading_HBlur;			// Shading horizontal blur Pass
		GLProgram *DOF_PRG_Shading_VBlur;			// Shading vertical blur Pass
		GLProgram *DOF_PRG_DepthOfField;			// Depth of Field Pass (Mix the blurred and original image according to focal range)

		GLProgram *FOG_PRG_Fog;						// Fog Pass (Change pixel color according to depth)

		GLProgram *MB_PRG_MotionBlur;

		GLProgram *TEST_PRG_Test;

		//----- FBOs -----//
		GPUFBO *Depth_FBO_Normalized;

		GPUFBO *DS_FBO_Shading;

		GPUFBO *AA_FBO_Edge;
		GPUFBO *AA_FBO_Edge_Blur[2];
		GPUFBO *AA_FBO_Shading_Blur[2];
		GPUFBO *AA_FBO_AntiAliasing;

		GPUFBO *DO_FBO_DO;
		GPUFBO *DO_FBO_DO_Blur[2];

		GPUFBO *TM_FBO_Luminance;
		GPUFBO *TM_FBO_Luminance_Downscale[6];
		GPUFBO *TM_FBO_ToneMapping;

		GPUFBO *Bl_FBO_Bright;
		GPUFBO *Bl_FBO_Bright_Blur[2];
		GPUFBO *Bl_FBO_Bloom;

		GPUFBO *DOF_FBO_Focal;
		GPUFBO *DOF_FBO_Focal_Blur[2];
		GPUFBO *DOF_FBO_Shading_Blur[2];
		GPUFBO *DOF_FBO_DepthOfField;

		GPUFBO *FOG_FBO_Fog;

		GPUFBO *MB_FBO_MotionBlur;

		GPUFBO *TEST_FBO_Test;
		

		//----- Samplers -----//
		GPUsampler *Depth_Sampler_Depth;

		GPUsampler *DS_Sampler_Shading_GBuffers;
		GPUsampler *DS_Sampler_Shading_AO;

		GPUsampler *AA_Sampler_Edge_EdgeDetection_GBuffers;	
		GPUsampler *AA_Sampler_Edge_EdgeDetection_Depth;		
		GPUsampler *AA_Sampler_Edge_HBlur_Edge;				
		GPUsampler *AA_Sampler_Edge_VBlur_Edge;		
		GPUsampler *AA_Sampler_Shading_HBlur_Edge;	
		GPUsampler *AA_Sampler_Shading_HBlur_Shading;	
		GPUsampler *AA_Sampler_Shading_VBlur_Edge;	
		GPUsampler *AA_Sampler_Shading_VBlur_Shading;					
		GPUsampler *AA_Sampler_AntiAliasing_Color;			
		GPUsampler *AA_Sampler_AntiAliasing_Edge;			
		GPUsampler *AA_Sampler_AntiAliasing_Blur;			

		GPUsampler *DO_Sampler_DO_GBuffers;
		GPUsampler *DO_Sampler_DO_Depth;
		GPUsampler *DO_Sampler_DO_Noise;
		GPUsampler *DO_Sampler_DO_HBlur_DO;
		GPUsampler *DO_Sampler_DO_HBLUR_GBuffers;
		GPUsampler *DO_Sampler_DO_VBlur_DO;
		GPUsampler *DO_Sampler_DO_VBLUR_GBuffers;

		GPUsampler *TM_Sampler_Luminance_Computation_HDRLuminance;	
		GPUsampler *TM_Sampler_Luminance_Downscale_Luminance;		
		GPUsampler *TM_Sampler_ToneMapping_HDR;				

		GPUsampler *Bl_Sampler_Bright_Computation_Color;		
		GPUsampler *Bl_Sampler_Bright_HBlur_Bright;				
		GPUsampler *Bl_Sampler_Bright_VBlur_Bright;				
		GPUsampler *Bl_Sampler_Bloom_Color;			
		GPUsampler *Bl_Sampler_Bloom_Blur;							

		GPUsampler *DOF_Sampler_Focal_Depth_Depth;				
		GPUsampler *DOF_Sampler_Focal_HBlur_Focal;				
		GPUsampler *DOF_Sampler_Focal_VBlur_Focal;				
		GPUsampler *DOF_Sampler_Shading_HBlur_Focal;			
		GPUsampler *DOF_Sampler_Shading_HBlur_Shading;			
		GPUsampler *DOF_Sampler_Shading_VBlur_Focal;				
		GPUsampler *DOF_Sampler_Shading_VBlur_Shading;				
		GPUsampler *DOF_Sampler_DepthOfField_Color;		
		GPUsampler *DOF_Sampler_DepthOfField_Blur;		
		GPUsampler *DOF_Sampler_DepthOfField_Focal;			

		GPUsampler *FOG_Sampler_Fog_Color;	
		GPUsampler *FOG_Sampler_Fog_Depth;			

		GPUsampler *MB_Sampler_MotionBlur_Color;
		//GPUsampler *MB_Sampler_MotionBlur_Depth;
		GPUsampler *MB_Sampler_MotionBlur_Velocity;

		GPUsampler *TEST_Sampler_Test_Color;
		GPUsampler *TEST_Sampler_Test_Depth;

		//----- GPU Parameters -----//
		GPUfloat *depthCameraNear;
		GPUfloat *depthCameraFar;

		GPUvec3 *dsCamPos;
		
		GPUfloat *aaEdgeHStep;
		GPUfloat *aaEdgeVStep;
		GPUfloat *aaEdgeHBlurHStep;
		GPUfloat *aaEdgeVBlurVStep;
		GPUfloat *aaShadingHBlurHStep;
		GPUfloat *aaShadingVBlurVStep;

		GPUfloat *doHStep;
		GPUfloat *doVStep;
		GPUfloat *doBaseRadius;
		GPUfloat *doMaxOcclusionDist;
		GPUint *numSamples;

		GPUvec3 *doCamPos;
		GPUint *doHBlurSize;
		GPUfloat *doHBlurHStep;
		GPUint *doVBlurSize;
		GPUfloat *doVBlurVStep;

		GLuint noiseSamplerID;
		GLuint noiseTextureID;
		GLuint noiseTextureBindChannel;

		GPUfloat *tmMaxRGBValue;
		GPUfloat *tmExposure;
		GPUfloat *tmLuminanceHStep;
		GPUfloat *tmLuminanceVStep;

		GPUfloat *blThreshold;
		GPUint *blHBlurSize;
		GPUfloat *blHBlurHStep;
		GPUint *blVBlurSize;
		GPUfloat *blVBlurVStep;

		GPUfloat *dofCameraNear;
		GPUfloat *dofCameraFar;

		GPUfloat *dofFocalNear;
		GPUfloat *dofFocalFar;
				
		GPUint *dofFocalHBlurSize;
		GPUfloat *dofFocalHBlurHStep;
		GPUint *dofFocalVBlurSize;
		GPUfloat *dofFocalVBlurVStep;
		GPUint *dofShadingHBlurSize;
		GPUfloat *dofShadingHBlurMultiplier;
		GPUfloat *dofShadingHBlurHStep;
		GPUint *dofShadingVBlurSize;
		GPUfloat *dofShadingVBlurMultiplier;
		GPUfloat *dofShadingVBlurVStep;

		GPUfloat *fogZNear;
		GPUfloat *fogZFar;

		//GPUmat4 *mbInvViewProjMat;
		//GPUmat4 *mbInvViewProjMatLast;
		GPUfloat *velocityScale;

		GPUfloat *testTime;
		
		
		//----- CPU Parameters -----//
		int tmNumDownscaleBuffers;		// Tone Mapping : Number of Buffers used for Luminance Downscale
		float tmWaitTime;				// Tone Mapping : Luminance computation time interval
		float tmLastTime;				// Tone Mapping : Last luminance computation time
		float tmTime;					// Tone Mapping : Current time
		float tmLastMaxLuminance;		// Tone Mapping : Last maximum luminance
		float tmLastPower, tmNewPower;	// Tone Mapping : Powers used in the weighted average new luminance
		float data[4096];				// Tone Mapping : Buffer to store 32x32 luminance texture

		float mbTime;
		float mbLastTime;
		int mbNbFrames = 0;

		std::chrono::high_resolution_clock::time_point lastUpdateTime;
		float lastDelta = 0.0f;

};
#endif