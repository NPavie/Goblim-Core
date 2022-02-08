#include "Effects/PostProcess/PostProcess.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Engine.h"
#include <time.h>
#include <iostream>
PostProcess::PostProcess(std::string name, 
						bool useHDRPostProcess ,
						bool useAA ,
						bool useDO,
						float numSamples,
						int noiseSize,
						bool useTM ,
						bool useTemporalTM ,
						float exposure,
						bool useBl ,
						float threshold,
						int blurSize,
						bool useDOF ,
						bool useNearDOF ,
						bool useFarDOF ,
						float nearFocal,
						float farFocal,
						int focalBlurSize,
						int shadingBlurSize,
						bool useFog,
						bool useMB,
						bool useFinal) : EffectGL(name, "PostProcess")
{
	this->useHDRPostProcess = useHDRPostProcess;
	this->useAA = useAA;
	this->useDO = useDO;
	this->useTM = useTM;
	this->useTemporalTM = useTemporalTM;
	this->useBl = useBl;
	this->useDOF = useDOF;
	this->useNearDOF = useNearDOF;
	this->useFarDOF = useFarDOF;
	this->useFog = useFog;
	this->useMB = useMB;
	this->useFinal = useFinal;

	tmNewPower = 0.5f;
	tmLastPower = 0.5f;
	tmLastTime = 0.0f;
	tmLastMaxLuminance = 1.0f;
	if(useTemporalTM)
		tmWaitTime = 0.04f; // 0.04f
	else
		tmWaitTime = 0.0f;

	this->createPrograms();
	this->createFBOs();
	this->createSamplers();
	this->createGPUParameters(threshold, blurSize, nearFocal, farFocal, focalBlurSize, shadingBlurSize, exposure);
	if ( useDO )
		this->computeAOPreProcess(numSamples, noiseSize);
}
PostProcess::~PostProcess()
{	
	
}

void PostProcess::reload (	float threshold,
							int blurSize,
							float nearFocal,
							float farFocal,
							int focalBlurSize,
							int shadingBlurSize,
							float numSamples,
							int noiseSize	) {

	
}

void PostProcess::bindNoiseTexture(GLuint channel)
{
	noiseTextureBindChannel = channel;
	GLuint target = GL_TEXTURE0 + channel;
	glActiveTexture(target);
	glBindTexture(GL_TEXTURE_2D, noiseTextureID);
	glBindSampler(channel, noiseSamplerID);
}
void PostProcess::releaseNoiseTexture()
{
	GLuint target = GL_TEXTURE0 + noiseTextureBindChannel;
	glActiveTexture(target);
	glBindTexture(noiseTextureID, 0);
	glBindSampler(noiseTextureBindChannel, 0);
}

void PostProcess::createPrograms()
{
	Base = new GLProgram(this->m_ClassName + "-Base", GL_VERTEX_SHADER);

	Depth_PRG_Normalize = new GLProgram(this->m_ClassName + "-Depth-Normalize", GL_FRAGMENT_SHADER);

	if ( !useDO )
		DS_PRG_Shading = new GLProgram(this->m_ClassName + "-DS-Shading", GL_FRAGMENT_SHADER);
	else
		DS_PRG_Shading = new GLProgram(this->m_ClassName + "-DS-Shading-AO", GL_FRAGMENT_SHADER);

	m_ProgramPipeline->useProgramStage(GL_VERTEX_SHADER_BIT, Base);

	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, Depth_PRG_Normalize);
	m_ProgramPipeline->link();

	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DS_PRG_Shading);
	m_ProgramPipeline->link();

	if(useAA)
	{		
		AA_PRG_Edge_EdgeDetection = new GLProgram(this->m_ClassName + "-AA-Edge-EdgeDetection", GL_FRAGMENT_SHADER);
		AA_PRG_Edge_HBlur = new GLProgram(this->m_ClassName + "-AA-Edge-HBlur", GL_FRAGMENT_SHADER);
		AA_PRG_Edge_VBlur = new GLProgram(this->m_ClassName + "-AA-Edge-VBlur", GL_FRAGMENT_SHADER);
		AA_PRG_Shading_HBlur = new GLProgram(this->m_ClassName + "-AA-Shading-HBlur", GL_FRAGMENT_SHADER);
		AA_PRG_Shading_VBlur = new GLProgram(this->m_ClassName + "-AA-Shading-VBlur", GL_FRAGMENT_SHADER);
		AA_PRG_AntiAliasing = new GLProgram(this->m_ClassName + "-AA-AntiAliasing", GL_FRAGMENT_SHADER);

		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, AA_PRG_Edge_EdgeDetection);
		m_ProgramPipeline->link();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, AA_PRG_Edge_HBlur);
		m_ProgramPipeline->link();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, AA_PRG_Edge_VBlur);
		m_ProgramPipeline->link();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, AA_PRG_Shading_HBlur);
		m_ProgramPipeline->link();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, AA_PRG_Shading_VBlur);
		m_ProgramPipeline->link();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, AA_PRG_AntiAliasing);
		m_ProgramPipeline->link();
	}

	if ( useDO )
	{
		DO_PRG_DO		= new GLProgram(this->m_ClassName + "-SSDO", GL_FRAGMENT_SHADER);
		DO_PRG_DO_HBlur = new GLProgram(this->m_ClassName + "-SSDO-HBlur", GL_FRAGMENT_SHADER);
		DO_PRG_DO_VBlur = new GLProgram(this->m_ClassName + "-SSDO-VBlur", GL_FRAGMENT_SHADER);

		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DO_PRG_DO);
		m_ProgramPipeline->link();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DO_PRG_DO_HBlur);
		m_ProgramPipeline->link();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DO_PRG_DO_VBlur);
		m_ProgramPipeline->link();
	}

	if(useTM)
	{
		TM_PRG_Luminance_Computation = new GLProgram(this->m_ClassName + "-TM-Luminance-Computation", GL_FRAGMENT_SHADER);
		TM_PRG_Luminance_Downscale = new GLProgram(this->m_ClassName + "-TM-Luminance-Downscale", GL_FRAGMENT_SHADER);
		TM_PRG_ToneMapping = new GLProgram(this->m_ClassName + "-TM-ToneMapping", GL_FRAGMENT_SHADER);

		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, TM_PRG_Luminance_Computation);
		m_ProgramPipeline->link();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, TM_PRG_Luminance_Downscale);
		m_ProgramPipeline->link();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, TM_PRG_ToneMapping);
		m_ProgramPipeline->link();
	}

	if(useBl)
	{
		Bl_PRG_Bright_Computation	= new GLProgram(this->m_ClassName + "-BL-Bright-Computation", GL_FRAGMENT_SHADER);
		Bl_PRG_Bright_HBlur			= new GLProgram(this->m_ClassName + "-BL-Bright-HBlur", GL_FRAGMENT_SHADER);
		Bl_PRG_Bright_VBlur			= new GLProgram(this->m_ClassName + "-BL-Bright-VBlur", GL_FRAGMENT_SHADER);
		Bl_PRG_Bloom				= new GLProgram(this->m_ClassName + "-BL-Bloom", GL_FRAGMENT_SHADER);

		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, Bl_PRG_Bright_Computation);
		m_ProgramPipeline->link();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, Bl_PRG_Bright_HBlur);
		m_ProgramPipeline->link();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, Bl_PRG_Bright_VBlur);
		m_ProgramPipeline->link();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, Bl_PRG_Bloom);
		m_ProgramPipeline->link();
	}

	if(useDOF)
	{
		if(useNearDOF && useFarDOF)
			DOF_PRG_Focal_Depth	= new GLProgram(this->m_ClassName + "-DOF-Focal-NearFarDepth", GL_FRAGMENT_SHADER);
		else
		{
			if(useNearDOF)
				DOF_PRG_Focal_Depth	= new GLProgram(this->m_ClassName + "-DOF-Focal-NearDepth", GL_FRAGMENT_SHADER);
			if(useFarDOF)
				DOF_PRG_Focal_Depth	= new GLProgram(this->m_ClassName + "-DOF-Focal-FarDepth", GL_FRAGMENT_SHADER);
		}

		DOF_PRG_Focal_HBlur		= new GLProgram(this->m_ClassName + "-DOF-Focal-HBlur", GL_FRAGMENT_SHADER);
		DOF_PRG_Focal_VBlur		= new GLProgram(this->m_ClassName + "-DOF-Focal-VBlur", GL_FRAGMENT_SHADER);
		DOF_PRG_Shading_HBlur	= new GLProgram(this->m_ClassName + "-DOF-Shading-HBlur", GL_FRAGMENT_SHADER);
		DOF_PRG_Shading_VBlur	= new GLProgram(this->m_ClassName + "-DOF-Shading-VBlur", GL_FRAGMENT_SHADER);
		DOF_PRG_DepthOfField	= new GLProgram(this->m_ClassName + "-DOF-DepthOfField", GL_FRAGMENT_SHADER);

		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DOF_PRG_Focal_Depth);
		m_ProgramPipeline->link();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DOF_PRG_Focal_HBlur);
		m_ProgramPipeline->link();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DOF_PRG_Focal_VBlur);
		m_ProgramPipeline->link();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DOF_PRG_Shading_HBlur);
		m_ProgramPipeline->link();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DOF_PRG_Shading_VBlur);
		m_ProgramPipeline->link();
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DOF_PRG_DepthOfField);
		m_ProgramPipeline->link();
	}

	if(useFog)
	{
		FOG_PRG_Fog	= new GLProgram(this->m_ClassName + "-FOG-Fog", GL_FRAGMENT_SHADER);

		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, FOG_PRG_Fog);
		m_ProgramPipeline->link();
	}	

	if (useMB)
	{
		/*MB_PRG_MotionBlur = new GLProgram (this->m_ClassName + "-MB-MotionBlur", GL_FRAGMENT_SHADER);
		m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, MB_PRG_MotionBlur);
		m_ProgramPipeline->link ();*/
		MB_PRG_MotionBlur = new GLProgram (this->m_ClassName + "-MB-MotionBlur-PerObject", GL_FRAGMENT_SHADER);
		m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, MB_PRG_MotionBlur);
		m_ProgramPipeline->link ();
	}

	if (useFinal)
	{
		TEST_PRG_Test = new GLProgram (this->m_ClassName + "-TEST-Test", GL_FRAGMENT_SHADER);
		m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, TEST_PRG_Test);
		m_ProgramPipeline->link ();
	}
	
}
void PostProcess::createFBOs()
{
	unsigned int FBOWidth, FBOHeight, i;

	Depth_FBO_Normalized = new GPUFBO("Normalized Depth Buffer");
	Depth_FBO_Normalized->create(FBO_WIDTH, FBO_HEIGHT, 1, false, GL_R32F, GL_TEXTURE_2D, 1);

	DS_FBO_Shading	= new GPUFBO("DS_FBO_Shading");
	DS_FBO_Shading->create(FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);

	if(useAA)
	{		
		AA_FBO_Edge				= new GPUFBO("AA_FBO_Edge");
		AA_FBO_Edge_Blur[0]		= new GPUFBO("AA_FBO_Edge_HBlur");
		AA_FBO_Edge_Blur[1]		= new GPUFBO("AA_FBO_Edge_VBlur");
		AA_FBO_Shading_Blur[0]	= new GPUFBO("AA_FBO_Shading_HBlur");
		AA_FBO_Shading_Blur[1]	= new GPUFBO("AA_FBO_Shading_VBlur");
		AA_FBO_AntiAliasing		= new GPUFBO("AA_FBO_AntiAliasing");

		AA_FBO_Edge->create(			FBO_WIDTH, FBO_HEIGHT, 1, false, GL_R16F, GL_TEXTURE_2D, 1);
		AA_FBO_Edge_Blur[0]->create(	FBO_WIDTH, FBO_HEIGHT, 1, false, GL_R16F, GL_TEXTURE_2D, 1);
		AA_FBO_Edge_Blur[1]->create(	FBO_WIDTH, FBO_HEIGHT, 1, false, GL_R16F, GL_TEXTURE_2D, 1);
		AA_FBO_Shading_Blur[0]->create(	FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
		AA_FBO_Shading_Blur[1]->create(	FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
		AA_FBO_AntiAliasing->create(	FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
	}

	if ( useDO )
	{
		DO_FBO_DO			= new GPUFBO("DO_FBO_DO");
		DO_FBO_DO_Blur[0]	= new GPUFBO("DO_FBO_DO_HBlur");
		DO_FBO_DO_Blur[1]	= new GPUFBO("DO_FBO_DO_VBlur");

		DO_FBO_DO->create(			FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
		DO_FBO_DO_Blur[0]->create(	FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
		DO_FBO_DO_Blur[1]->create(	FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
	}

	if(useTM)
	{
		i = 0;
		FBOWidth =	FBO_WIDTH / 2;
		FBOHeight = FBO_HEIGHT / 2;

		TM_FBO_Luminance 	= new GPUFBO("TM_FBO_Luminance");
		TM_FBO_ToneMapping 	= new GPUFBO("TM_FBO_Luminance");

		TM_FBO_Luminance->create(	FBO_WIDTH, FBO_HEIGHT, 1, false, GL_R16F, GL_TEXTURE_2D, 1);
		TM_FBO_ToneMapping->create(	FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);

		TM_FBO_Luminance_Downscale[6];
		do
		{
			TM_FBO_Luminance_Downscale[i] = new GPUFBO("TM_FBO_Luminance_Downscale");
			TM_FBO_Luminance_Downscale[i]->create(	FBOWidth, FBOHeight, 1, false, GL_R16F, GL_TEXTURE_2D, 1);
			
			FBOWidth = FBOWidth / 2;
			FBOHeight = FBOHeight / 2;

			++i;
		}
		while(FBOWidth > 32 || FBOHeight > 32);

		tmNumDownscaleBuffers = i;
	}

	if(useBl)
	{
		Bl_FBO_Bright			= new GPUFBO("Bl_FBO_Bright");
		Bl_FBO_Bright_Blur[0]	= new GPUFBO("Bl_FBO_Bright_HBlur");
		Bl_FBO_Bright_Blur[1]	= new GPUFBO("Bl_FBO_Bright_VBlur");
		Bl_FBO_Bloom			= new GPUFBO("Bl_FBO_Bloom");

		Bl_FBO_Bright->create(			FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
		Bl_FBO_Bright_Blur[0]->create(	FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
		Bl_FBO_Bright_Blur[1]->create(	FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
		Bl_FBO_Bloom->create(			FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
	}

	if(useDOF)
	{
		DOF_FBO_Focal 			= new GPUFBO("DOF_FBO_Focal");
		DOF_FBO_Focal_Blur[0]	= new GPUFBO("DOF_FBO_Focal_HBlur");
		DOF_FBO_Focal_Blur[1]	= new GPUFBO("DOF_FBO_Focal_VBlur");
		DOF_FBO_Shading_Blur[0]	= new GPUFBO("DOF_FBO_Shading_HBlur");
		DOF_FBO_Shading_Blur[1]	= new GPUFBO("DOF_FBO_Shading_VBlur");
		DOF_FBO_DepthOfField	= new GPUFBO("DOF_FBO_DepthOfField");


		DOF_FBO_Focal->create(			FBO_WIDTH, FBO_HEIGHT, 1, false, GL_R16F, GL_TEXTURE_2D, 1);
		DOF_FBO_Focal_Blur[0]->create(	FBO_WIDTH, FBO_HEIGHT, 1, false, GL_R16F, GL_TEXTURE_2D, 1);
		DOF_FBO_Focal_Blur[1]->create(	FBO_WIDTH, FBO_HEIGHT, 1, false, GL_R16F, GL_TEXTURE_2D, 1);
		DOF_FBO_Shading_Blur[0]->create(FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
		DOF_FBO_Shading_Blur[1]->create(FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
		DOF_FBO_DepthOfField->create(	FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
	}

	if(useFog)
	{
		FOG_FBO_Fog	= new GPUFBO("FOG_FBO_Fog");
		FOG_FBO_Fog->create(FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
	}

	if (useMB)
	{
		MB_FBO_MotionBlur = new GPUFBO ("MB_FBO_MotionBlur");
		MB_FBO_MotionBlur->create (FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
	}

	if (useFinal)
	{
		TEST_FBO_Test = new GPUFBO ("TEST_FBO_Test");
		TEST_FBO_Test->create (FBO_WIDTH, FBO_HEIGHT, 1, false, GL_RGB16F, GL_TEXTURE_2D, 1);
	}

}
void PostProcess::createSamplers()
{
	Depth_Sampler_Depth = Depth_PRG_Normalize->uniforms()->getGPUsampler("DepthSampler");
	Depth_Sampler_Depth->Set(0);

	DS_Sampler_Shading_GBuffers = DS_PRG_Shading->uniforms()->getGPUsampler("GBuffers");
	DS_Sampler_Shading_GBuffers->Set(0);
	if ( useDO )
	{
		DS_Sampler_Shading_AO = DS_PRG_Shading->uniforms()->getGPUsampler("DOSampler");
		DS_Sampler_Shading_AO->Set(1);
	}

	if(useAA)
	{		
		AA_Sampler_Edge_EdgeDetection_GBuffers	= AA_PRG_Edge_EdgeDetection->uniforms()->getGPUsampler("GBuffers");
		AA_Sampler_Edge_EdgeDetection_Depth 	= AA_PRG_Edge_EdgeDetection->uniforms()->getGPUsampler("DepthSampler");
		AA_Sampler_Edge_HBlur_Edge 				= AA_PRG_Edge_HBlur->uniforms()->getGPUsampler("EdgeSampler");
		AA_Sampler_Edge_VBlur_Edge 				= AA_PRG_Edge_VBlur->uniforms()->getGPUsampler("EdgeSampler");
		AA_Sampler_Shading_HBlur_Edge 			= AA_PRG_Shading_HBlur->uniforms()->getGPUsampler("EdgeSampler");
		AA_Sampler_Shading_HBlur_Shading 		= AA_PRG_Shading_HBlur->uniforms()->getGPUsampler("ShadingSampler");
		AA_Sampler_Shading_VBlur_Edge 			= AA_PRG_Shading_VBlur->uniforms()->getGPUsampler("EdgeSampler");
		AA_Sampler_Shading_VBlur_Shading 		= AA_PRG_Shading_VBlur->uniforms()->getGPUsampler("ShadingSampler");
		AA_Sampler_AntiAliasing_Color 			= AA_PRG_AntiAliasing->uniforms()->getGPUsampler("ColorSampler");
		AA_Sampler_AntiAliasing_Edge 			= AA_PRG_AntiAliasing->uniforms()->getGPUsampler("EdgeSampler");
		AA_Sampler_AntiAliasing_Blur 			= AA_PRG_AntiAliasing->uniforms()->getGPUsampler("BlurSampler");

		AA_Sampler_Edge_EdgeDetection_GBuffers->Set(0);
		AA_Sampler_Edge_EdgeDetection_Depth->Set(1);
		AA_Sampler_Edge_HBlur_Edge->Set(0);
		AA_Sampler_Edge_VBlur_Edge->Set(0);
		AA_Sampler_Shading_HBlur_Edge->Set(0);
		AA_Sampler_Shading_HBlur_Shading->Set(1);
		AA_Sampler_Shading_VBlur_Edge->Set(0);
		AA_Sampler_Shading_VBlur_Shading->Set(1);
		AA_Sampler_AntiAliasing_Color->Set(0);
		AA_Sampler_AntiAliasing_Edge->Set(1);
		AA_Sampler_AntiAliasing_Blur->Set(2);
	}

	if ( useDO )
	{
		DO_Sampler_DO_GBuffers	= DO_PRG_DO->uniforms()->getGPUsampler("GBuffers");
		DO_Sampler_DO_Depth		= DO_PRG_DO->uniforms()->getGPUsampler("depthSampler");
		DO_Sampler_DO_Noise		= DO_PRG_DO->uniforms()->getGPUsampler("noiseSampler");

		
		DO_Sampler_DO_HBLUR_GBuffers = DO_PRG_DO_HBlur->uniforms()->getGPUsampler( "GBuffers" );
		DO_Sampler_DO_HBlur_DO = DO_PRG_DO_HBlur->uniforms()->getGPUsampler( "DOSampler" );
		DO_Sampler_DO_VBLUR_GBuffers = DO_PRG_DO_VBlur->uniforms()->getGPUsampler( "GBuffers" );
		DO_Sampler_DO_VBlur_DO = DO_PRG_DO_VBlur->uniforms()->getGPUsampler( "DOSampler" );

		DO_Sampler_DO_GBuffers->Set(0);
		DO_Sampler_DO_Depth->Set(1);
		DO_Sampler_DO_Noise->Set(2);

		DO_Sampler_DO_HBLUR_GBuffers->Set( 0 );
		DO_Sampler_DO_HBlur_DO->Set(1);
		DO_Sampler_DO_VBLUR_GBuffers->Set( 0 );
		DO_Sampler_DO_VBlur_DO->Set( 1 );
	}

	if(useTM)
	{
		TM_Sampler_Luminance_Computation_HDRLuminance 	= TM_PRG_Luminance_Computation->uniforms()->getGPUsampler("HDRLuminanceSampler");
		TM_Sampler_Luminance_Downscale_Luminance		= TM_PRG_Luminance_Downscale->uniforms()->getGPUsampler("LuminanceSampler");
		TM_Sampler_ToneMapping_HDR						= TM_PRG_ToneMapping->uniforms()->getGPUsampler("HDRSampler");
	
		TM_Sampler_Luminance_Computation_HDRLuminance->Set(0);
		TM_Sampler_Luminance_Downscale_Luminance->Set(0);
		TM_Sampler_ToneMapping_HDR->Set(0);
	}

	if(useBl)
	{
		Bl_Sampler_Bright_Computation_Color	= Bl_PRG_Bright_Computation->uniforms()->getGPUsampler("ColorSampler");
		Bl_Sampler_Bright_HBlur_Bright		= Bl_PRG_Bright_HBlur->uniforms()->getGPUsampler("BrightSampler");
		Bl_Sampler_Bright_VBlur_Bright		= Bl_PRG_Bright_VBlur->uniforms()->getGPUsampler("BrightSampler");
		Bl_Sampler_Bloom_Color				= Bl_PRG_Bloom->uniforms()->getGPUsampler("ColorSampler");
		Bl_Sampler_Bloom_Blur				= Bl_PRG_Bloom->uniforms()->getGPUsampler("BlurSampler");

		Bl_Sampler_Bright_Computation_Color->Set(0);
		Bl_Sampler_Bright_HBlur_Bright->Set(0);
		Bl_Sampler_Bright_VBlur_Bright->Set(0);
		Bl_Sampler_Bloom_Color->Set(0);
		Bl_Sampler_Bloom_Blur->Set(1);
	}

	if(useDOF)
	{
		DOF_Sampler_Focal_Depth_Depth		= DOF_PRG_Focal_Depth->uniforms()->getGPUsampler("DepthSampler");
		DOF_Sampler_Focal_HBlur_Focal		= DOF_PRG_Focal_HBlur->uniforms()->getGPUsampler("FocalSampler");
		DOF_Sampler_Focal_VBlur_Focal		= DOF_PRG_Focal_VBlur->uniforms()->getGPUsampler("FocalSampler");
		DOF_Sampler_Shading_HBlur_Focal		= DOF_PRG_Shading_HBlur->uniforms()->getGPUsampler("FocalSampler");
		DOF_Sampler_Shading_HBlur_Shading	= DOF_PRG_Shading_HBlur->uniforms()->getGPUsampler("ShadingSampler");
		DOF_Sampler_Shading_VBlur_Focal		= DOF_PRG_Shading_VBlur->uniforms()->getGPUsampler("FocalSampler");
		DOF_Sampler_Shading_VBlur_Shading	= DOF_PRG_Shading_VBlur->uniforms()->getGPUsampler("ShadingSampler");	
		DOF_Sampler_DepthOfField_Color		= DOF_PRG_DepthOfField->uniforms()->getGPUsampler("ColorSampler");
		DOF_Sampler_DepthOfField_Blur		= DOF_PRG_DepthOfField->uniforms()->getGPUsampler("BlurSampler");
		DOF_Sampler_DepthOfField_Focal		= DOF_PRG_DepthOfField->uniforms()->getGPUsampler("FocalSampler");

		DOF_Sampler_Focal_Depth_Depth->Set(0);
		DOF_Sampler_Focal_HBlur_Focal->Set(0);
		DOF_Sampler_Focal_VBlur_Focal->Set(0);
		DOF_Sampler_Shading_HBlur_Focal->Set(0);
		DOF_Sampler_Shading_HBlur_Shading->Set(1);
		DOF_Sampler_Shading_VBlur_Focal->Set(0);
		DOF_Sampler_Shading_VBlur_Shading->Set(1);
		DOF_Sampler_DepthOfField_Color->Set(0);
		DOF_Sampler_DepthOfField_Blur->Set(1);
		DOF_Sampler_DepthOfField_Focal->Set(2);
	}

	if(useFog)
	{
		FOG_Sampler_Fog_Color	= FOG_PRG_Fog->uniforms()->getGPUsampler("ColorSampler");
		FOG_Sampler_Fog_Depth	= FOG_PRG_Fog->uniforms()->getGPUsampler("DepthSampler");

		FOG_Sampler_Fog_Color->Set(0);
		FOG_Sampler_Fog_Depth->Set(1);
	}

	if (useMB) {
		/*MB_Sampler_MotionBlur_Color = MB_PRG_MotionBlur->uniforms ()->getGPUsampler ("ColorSampler");
		MB_Sampler_MotionBlur_Color->Set (0);
		MB_Sampler_MotionBlur_Depth = MB_PRG_MotionBlur->uniforms ()->getGPUsampler ("DepthSampler");
		MB_Sampler_MotionBlur_Depth->Set (1);*/
		MB_Sampler_MotionBlur_Color = MB_PRG_MotionBlur->uniforms ()->getGPUsampler ("ColorSampler");
		MB_Sampler_MotionBlur_Color->Set (0);
		MB_Sampler_MotionBlur_Velocity = MB_PRG_MotionBlur->uniforms ()->getGPUsampler ("GBuffers");
		MB_Sampler_MotionBlur_Velocity->Set (1);
	}

	if (useFinal)
	{
		TEST_Sampler_Test_Color = TEST_PRG_Test->uniforms ()->getGPUsampler ("ColorSampler");
		TEST_Sampler_Test_Color->Set (0);
		TEST_Sampler_Test_Depth = TEST_PRG_Test->uniforms ()->getGPUsampler ("DepthSampler");
		TEST_Sampler_Test_Depth->Set (1);
	}

}
void PostProcess::createGPUParameters(float threshold, int blurSize, float nearFocal, float farFocal, int focalBlurSize, int shadingBlurSize, float exposure)
{
	depthCameraNear = Depth_PRG_Normalize->uniforms()->getGPUfloat("zNear");
	depthCameraFar = Depth_PRG_Normalize->uniforms()->getGPUfloat("zFar");

	depthCameraNear->Set(Scene::getInstance()->camera()->getZnear());
	depthCameraFar->Set(Scene::getInstance()->camera()->getZfar());

	dsCamPos = DS_PRG_Shading->uniforms()->getGPUvec3("CamPos");

	if(useAA)
	{		
		aaEdgeHStep			= AA_PRG_Edge_EdgeDetection->uniforms()->getGPUfloat("HStep");
		aaEdgeVStep			= AA_PRG_Edge_EdgeDetection->uniforms()->getGPUfloat("VStep");
		aaEdgeHBlurHStep	= AA_PRG_Edge_HBlur->uniforms()->getGPUfloat("HStep");
		aaEdgeVBlurVStep	= AA_PRG_Edge_VBlur->uniforms()->getGPUfloat("VStep");	
		aaShadingHBlurHStep	= AA_PRG_Shading_HBlur->uniforms()->getGPUfloat("HStep");
		aaShadingVBlurVStep = AA_PRG_Shading_VBlur->uniforms()->getGPUfloat("VStep");

		aaEdgeHStep->Set(1.0f / AA_FBO_Edge->getWidth());
		aaEdgeVStep->Set(1.0f / AA_FBO_Edge->getHeight());
		aaEdgeHBlurHStep->Set(1.0f / AA_FBO_Edge_Blur[0]->getWidth());
		aaEdgeVBlurVStep->Set(1.0f / AA_FBO_Edge_Blur[1]->getHeight());
		aaShadingHBlurHStep->Set(1.0f / AA_FBO_Shading_Blur[0]->getWidth());
		aaShadingVBlurVStep->Set(1.0f / AA_FBO_Shading_Blur[1]->getHeight());
	}

	if ( useDO )
	{
		doHStep = DO_PRG_DO->uniforms()->getGPUfloat( "HStep" );
		doVStep = DO_PRG_DO->uniforms()->getGPUfloat( "VStep" );
		numSamples = DO_PRG_DO->uniforms()->getGPUint( "numSamples" );

		doBaseRadius = DO_PRG_DO->uniforms()->getGPUfloat( "baseRadius" );
		doMaxOcclusionDist = DO_PRG_DO->uniforms()->getGPUfloat( "maxOcclusionDist" );

		doHBlurSize = DO_PRG_DO_HBlur->uniforms()->getGPUint( "doBlurSize" );
		doHBlurHStep = DO_PRG_DO_HBlur->uniforms()->getGPUfloat( "HStep" );

		doVBlurSize = DO_PRG_DO_VBlur->uniforms()->getGPUint("doBlurSize");
		doVBlurVStep = DO_PRG_DO_VBlur->uniforms()->getGPUfloat( "VStep" );
		
		doHBlurHStep->Set(1.0f / DO_FBO_DO_Blur[0]->getWidth());
		doVBlurVStep->Set(1.0f / DO_FBO_DO_Blur[1]->getHeight());
	}

	if(useTM)
	{
		tmLuminanceHStep 	= TM_PRG_Luminance_Downscale->uniforms()->getGPUfloat("HStep");
		tmLuminanceVStep 	= TM_PRG_Luminance_Downscale->uniforms()->getGPUfloat("VStep");
		tmMaxRGBValue 		= TM_PRG_ToneMapping->uniforms()->getGPUfloat("tmMaxRGBValue");
		tmExposure			= TM_PRG_ToneMapping->uniforms ()->getGPUfloat ("tmExposure");
		tmExposure->Set (exposure);
	}

	if(useBl)
	{
		blThreshold 	= Bl_PRG_Bright_Computation->uniforms()->getGPUfloat("Threshold");
		blHBlurSize		= Bl_PRG_Bright_HBlur->uniforms()->getGPUint("blBlurSize");
		blHBlurHStep	= Bl_PRG_Bright_HBlur->uniforms()->getGPUfloat("HStep");	
		blVBlurSize		= Bl_PRG_Bright_VBlur->uniforms()->getGPUint("blBlurSize");
		blVBlurVStep	= Bl_PRG_Bright_VBlur->uniforms()->getGPUfloat("VStep");	

		blThreshold->Set(threshold);
		blHBlurSize->Set(blurSize);
		blHBlurHStep->Set(1.0f / Bl_FBO_Bright_Blur[0]->getWidth());
		blVBlurSize->Set(blurSize);
		blVBlurVStep->Set(1.0f / Bl_FBO_Bright_Blur[1]->getHeight());
	}

	if(useDOF)
	{
		dofCameraNear	= DOF_PRG_Focal_Depth->uniforms()->getGPUfloat("zNear");
		dofCameraFar 	= DOF_PRG_Focal_Depth->uniforms()->getGPUfloat("zFar");
		if(useNearDOF && useFarDOF)
		{
			dofFocalNear 	= DOF_PRG_Focal_Depth->uniforms()->getGPUfloat("FocalNear");
			dofFocalFar		= DOF_PRG_Focal_Depth->uniforms()->getGPUfloat("FocalFar");
			dofFocalNear->Set(nearFocal);
			dofFocalFar->Set(farFocal);
		}
		else
		{
			if(useNearDOF)
			{
				dofFocalNear 	= DOF_PRG_Focal_Depth->uniforms()->getGPUfloat("FocalNear");
				dofFocalNear->Set(nearFocal);
			}
			if(useFarDOF)
			{
				dofFocalFar		= DOF_PRG_Focal_Depth->uniforms()->getGPUfloat("FocalFar");
				dofFocalFar->Set(farFocal);
			}
		}		
		dofFocalHBlurSize			= DOF_PRG_Focal_HBlur->uniforms()->getGPUint("dofFocalBlurSize");
		dofFocalHBlurHStep			= DOF_PRG_Focal_HBlur->uniforms()->getGPUfloat("HStep");
		dofFocalVBlurSize			= DOF_PRG_Focal_VBlur->uniforms()->getGPUint("dofFocalBlurSize");
		dofFocalVBlurVStep			= DOF_PRG_Focal_VBlur->uniforms()->getGPUfloat("VStep");	
		dofShadingHBlurSize			= DOF_PRG_Shading_HBlur->uniforms()->getGPUint("dofShadingBlurSize");
		//dofShadingHBlurMultiplier	= DOF_PRG_Shading_HBlur->uniforms()->getGPUfloat("dofFocalMultiplier");
		dofShadingHBlurHStep		= DOF_PRG_Shading_HBlur->uniforms()->getGPUfloat("HStep");
		dofShadingVBlurSize			= DOF_PRG_Shading_VBlur->uniforms()->getGPUint("dofShadingBlurSize");
		//dofShadingVBlurMultiplier	= DOF_PRG_Shading_VBlur->uniforms()->getGPUfloat("dofFocalMultiplier");
		dofShadingVBlurVStep		= DOF_PRG_Shading_VBlur->uniforms()->getGPUfloat("VStep");

		dofCameraNear->Set(Scene::getInstance()->camera()->getZnear());
		dofCameraFar->Set(Scene::getInstance()->camera()->getZfar());
		dofFocalHBlurSize->Set(focalBlurSize);
		dofFocalHBlurHStep->Set(1.0f / DOF_FBO_Focal_Blur[0]->getWidth());
		dofFocalVBlurSize->Set(focalBlurSize);
		dofFocalVBlurVStep->Set(1.0f / DOF_FBO_Focal_Blur[1]->getHeight());
		dofShadingHBlurSize->Set(shadingBlurSize);
		dofShadingHBlurHStep->Set(1.0f / DOF_FBO_Shading_Blur[0]->getWidth());
		dofShadingVBlurSize->Set(shadingBlurSize);
		dofShadingVBlurVStep->Set(1.0f / DOF_FBO_Shading_Blur[1]->getHeight());
	}

	if(useFog)
	{
		fogZNear= FOG_PRG_Fog->uniforms()->getGPUfloat("zNear");
		fogZFar	= FOG_PRG_Fog->uniforms()->getGPUfloat("zFar");

		fogZNear->Set(Scene::getInstance()->camera()->getZnear());
		fogZFar->Set(Scene::getInstance()->camera()->getZfar());
	}	

	if (useMB) 
	{
		/*mbInvViewProjMat = MB_PRG_MotionBlur->uniforms ()->getGPUmat4 ("invViewProj");
		mbInvViewProjMatLast = MB_PRG_MotionBlur->uniforms ()->getGPUmat4 ("invViewProjLast");*/
		velocityScale = MB_PRG_MotionBlur->uniforms ()->getGPUfloat ("velocityScale");
		velocityScale->Set (1.0f);
		mbTime = 0.0f;
		mbLastTime = 0.0f;
	}

	if (useFinal)
	{
		testTime = TEST_PRG_Test->uniforms ()->getGPUfloat ("time");
		testTime->Set (0.0f);
	}
}

void PostProcess::setExposure (float e) {
	tmExposure->Set (e);
}

void PostProcess::computeAOPreProcess(int Samples, int noiseSizeXY)
{	
	//aoNoiseTexture = Scene::getInstance()->getResource<GPUTexture2D>(ressourceTexPath + "noise.png");

	srand(NULL);

	//----- Samples Kernel Generation -----//
	float scale;
	unsigned int kernelSize = Samples;
	glm::vec3 *kernel = new glm::vec3[kernelSize];

	for (unsigned int i = 0; i < kernelSize; ++i)
	{
		// Generating random points in z oriented hemisphere
		kernel[i].x = ((-100.0f + (float)(rand() % 200)) / 100.0f);
		kernel[i].y = ((-100.0f + (float)(rand() % 200)) / 100.0f);
		kernel[i].z = ((-100.0f + (float)(rand() % 200)) / 100.0f);

		// Normalize the random vector to fall on the unit hemisphere
		kernel[i] = glm::normalize(kernel[i]);

		// Scale the random unit vector to fall randomly into the unit hemisphere
		//kernel[i] *= (((float)(rand() % 100)) / 100.0f);

		// Scale the random unit vector to fall randomly (but closer to the origin) into the unit hemisphere
		scale = float(i) / float(kernelSize);		
		scale = glm::mix(0.1f, 1.0f, scale * scale);
		kernel[i] *= scale;
	}

	// Send the samples kernel to the shader
	glUseProgram(DO_PRG_DO->getProgram());
	glUniform3fv(glGetUniformLocation(DO_PRG_DO->getProgram(), "kernel"), kernelSize, (float*)kernel);
	glUseProgram(0);

	delete[] kernel;

	this->numSamples->Set(Samples);

	//----- Noise Texture Generation -----//
	unsigned int noiseTexSizeX = noiseSizeXY;
	unsigned int noiseTexSizeY = noiseSizeXY;
	unsigned int noiseSize = noiseTexSizeX * noiseTexSizeY;
	glm::vec3 *noise = new glm::vec3[noiseSize];

	float thresold = 0.125f;
	for (unsigned int i = 0; i < noiseSize; ++i) 
	{
		// Generating random points in z oriented hemisphere
		noise[i].x = ((-1000.0f + (float)(rand() % 2000)) / 1000.0f);
		noise[i].y = ((-1000.0f + (float)(rand() % 2000)) / 1000.0f);
		noise[i].z = 0.0f;

		// Normalize the random vector to fall on the unit hemisphere
		noise[i] = glm::normalize(noise[i]);

		if ((noise[i].x > -thresold && noise[i].x < thresold) || (noise[i].y > -thresold && noise[i].y < thresold))
			i = --i;
	}
	
	// Set Up noise texture
	glGenTextures(1, &noiseTextureID);
	glBindTexture(GL_TEXTURE_2D, noiseTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, noiseTexSizeX, noiseTexSizeY, 0, GL_RGB, GL_FLOAT, (float*)noise);

	// Set Up noise sampler
	glGenSamplers(1, &noiseSamplerID);
	glBindSampler(noiseTextureID, noiseSamplerID);
	glSamplerParameteri(noiseSamplerID, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glSamplerParameteri(noiseSamplerID, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glSamplerParameteri(noiseSamplerID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(noiseSamplerID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	
	doHStep->Set(DO_FBO_DO->getWidth() / (float)noiseTexSizeX);
	doVStep->Set(DO_FBO_DO->getHeight() / (float)noiseTexSizeY);
	doHBlurSize->Set(noiseTexSizeX);
	doVBlurSize->Set(noiseTexSizeY);

	delete[] noise;
}

void PostProcess::draw()
{
	m_ProgramPipeline->bind();
		quad->drawGeometry(GL_TRIANGLES);
	m_ProgramPipeline->release();
}

// In FBO contains populated G-Buffer
void PostProcess::apply(GPUFBO *GBuffers, GPUFBO *out)
{	
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glClear(GL_COLOR_BUFFER_BIT);
	
	if (m_ProgramPipeline)
	{
		this->applyHDRPostProcess(GBuffers, out);			
	}
	glPopAttrib();
}

void PostProcess::applyHDRPostProcess(GPUFBO *GBuffers, GPUFBO *out)
{
	applyDepthLinear (GBuffers, Depth_FBO_Normalized);

	if ( useDO )
	{
		applyDirectionalOcclusion( GBuffers, Depth_FBO_Normalized, DO_FBO_DO_Blur[1] );
	}

	if ( !useDO )
		applyDeferredShading( GBuffers, NULL, DS_FBO_Shading );
	if ( useDO )
		applyDeferredShading( GBuffers, DO_FBO_DO_Blur[1], DS_FBO_Shading );
	
	if ( useAA )
		applyAntiAliasing (GBuffers, Depth_FBO_Normalized, DS_FBO_Shading, AA_FBO_AntiAliasing);

	if (useMB) {
		if (useAA)
			applyMotionBlur (GBuffers, AA_FBO_AntiAliasing, MB_FBO_MotionBlur);
		else
			applyMotionBlur (GBuffers, DS_FBO_Shading, MB_FBO_MotionBlur);
	}
	if (useBl)
	{
		if (useMB)
			applyBloom (MB_FBO_MotionBlur, Bl_FBO_Bloom);
		else {
			if (useAA) {
				applyBloom (AA_FBO_AntiAliasing, Bl_FBO_Bloom);
			}
			else
				applyBloom (DS_FBO_Shading, Bl_FBO_Bloom);
		}
	}

	if (useDOF)
	{
		if (useBl) {
			applyDepthOfField (GBuffers, Bl_FBO_Bloom, DOF_FBO_DepthOfField);
		}
		else {
			if (useMB)
				applyDepthOfField (GBuffers, MB_FBO_MotionBlur, DOF_FBO_DepthOfField);
			else
			{
				if (useAA)
					applyDepthOfField (GBuffers, AA_FBO_AntiAliasing, DOF_FBO_DepthOfField);
				else
					applyDepthOfField (GBuffers, DS_FBO_Shading, DOF_FBO_DepthOfField);
			}
		}
	}

	if (useFog)
	{
		if (useDOF)
			applyFog (GBuffers, DOF_FBO_DepthOfField, FOG_FBO_Fog);
		else
		{
			if (useBl)
				applyFog (GBuffers, Bl_FBO_Bloom, FOG_FBO_Fog);
			
			else {
				if (useMB)
					applyFog (GBuffers, MB_FBO_MotionBlur, FOG_FBO_Fog);
				else
				{
					if (useAA)
						applyFog (GBuffers, AA_FBO_AntiAliasing, FOG_FBO_Fog);
					else
					{
						applyFog (GBuffers, DS_FBO_Shading, FOG_FBO_Fog);
					}
				}
			}
		}
	}

	if (useFog)
	{
		applyToneMapping (FOG_FBO_Fog, out);
	}
	else
	{
		if (useDOF)
			applyToneMapping (DOF_FBO_DepthOfField, out);
		else
		{
			if (useBl)
				applyToneMapping (Bl_FBO_Bloom, out);
			else {
				if (useMB)
					applyToneMapping (MB_FBO_MotionBlur, out);
				else {
					if (useAA)
						applyToneMapping (AA_FBO_AntiAliasing, out);
					else {
						applyToneMapping (DS_FBO_Shading, out);
					}
				}
			}
		}
	}
}

void PostProcess::applyDepthLinear(GPUFBO *GBuffers, GPUFBO *out)
{
	//----- Depth Linearization -----//
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, Depth_PRG_Normalize);

	out->enable();
		GBuffers->getDepthTexture()->bind(Depth_Sampler_Depth->getValue());
			this->draw();
		GBuffers->getDepthTexture()->release();
	out->disable();
}
void PostProcess::applyDeferredShading(GPUFBO *GBuffers, GPUFBO *AO, GPUFBO *out)
{
	bool ao = useDO;

	//----- Deferred Shading -----//
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DS_PRG_Shading);
	dsCamPos->Set(Scene::getInstance()->camera()->convertPtTo(glm::vec3(0.0), Scene::getInstance()->getRoot()->frame()));

	out->enable();
		GBuffers->getColorTexture(0)->bind(DS_Sampler_Shading_GBuffers->getValue());
		if (ao)
			AO->getColorTexture(0)->bind(DS_Sampler_Shading_AO->getValue());
			this->draw();
		if (ao)
			AO->getColorTexture(0)->release();
		GBuffers->getColorTexture(0)->release();
	out->disable();
}
void PostProcess::applyAntiAliasing(GPUFBO *GBuffers, GPUFBO *Depth_FBO_Normalized, GPUFBO *Shading, GPUFBO *out)
{
	//----- Edge Detection -----//
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, AA_PRG_Edge_EdgeDetection);

	AA_FBO_Edge->enable();
		GBuffers->getColorTexture(0)->bind(AA_Sampler_Edge_EdgeDetection_GBuffers->getValue());
		Depth_FBO_Normalized->getColorTexture(0)->bind(AA_Sampler_Edge_EdgeDetection_Depth->getValue());
			this->draw();
		Depth_FBO_Normalized->getColorTexture(0)->release();
		GBuffers->getColorTexture(0)->release();
	AA_FBO_Edge->disable();

	//----- Edge Blur Pass -----//
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, AA_PRG_Edge_HBlur);

	AA_FBO_Edge_Blur[0]->enable();
		AA_FBO_Edge->getColorTexture(0)->bind(AA_Sampler_Edge_HBlur_Edge->getValue());
			this->draw();
		AA_FBO_Edge->getColorTexture(0)->release();
	AA_FBO_Edge_Blur[0]->disable();

	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, AA_PRG_Edge_VBlur);

	AA_FBO_Edge_Blur[1]->enable();
		AA_FBO_Edge_Blur[0]->getColorTexture(0)->bind(AA_Sampler_Edge_VBlur_Edge->getValue());
			this->draw();
		AA_FBO_Edge_Blur[0]->getColorTexture(0)->release();
	AA_FBO_Edge_Blur[1]->disable();

	//----- Shading Blur Pass -----//
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, AA_PRG_Shading_HBlur);

	AA_FBO_Shading_Blur[0]->enable();
		Shading->getColorTexture(0)->bind(AA_Sampler_Shading_HBlur_Shading->getValue());
		AA_FBO_Edge_Blur[1]->getColorTexture(0)->bind(AA_Sampler_Shading_HBlur_Edge->getValue());
			this->draw();
		AA_FBO_Edge_Blur[1]->getColorTexture(0)->release();
		Shading->getColorTexture(0)->release();
	AA_FBO_Shading_Blur[0]->disable();

	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, AA_PRG_Shading_VBlur);

	AA_FBO_Shading_Blur[1]->enable();
		AA_FBO_Shading_Blur[0]->getColorTexture(0)->bind(AA_Sampler_Shading_VBlur_Shading->getValue());
		AA_FBO_Edge_Blur[1]->getColorTexture(0)->bind(AA_Sampler_Shading_VBlur_Edge->getValue());
			this->draw();
		AA_FBO_Edge_Blur[1]->getColorTexture(0)->release();
		AA_FBO_Shading_Blur[0]->getColorTexture(0)->release();
	AA_FBO_Shading_Blur[1]->disable();

	//----- AntiAliasing Pass -----//
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, AA_PRG_AntiAliasing);

	out->enable();
		Shading->getColorTexture(0)->bind(AA_Sampler_AntiAliasing_Color->getValue());
		AA_FBO_Edge_Blur[1]->getColorTexture(0)->bind(AA_Sampler_AntiAliasing_Edge->getValue());
		AA_FBO_Shading_Blur[1]->getColorTexture(0)->bind(AA_Sampler_AntiAliasing_Blur->getValue());
			this->draw();
		AA_FBO_Shading_Blur[1]->getColorTexture(0)->release();
		AA_FBO_Edge_Blur[1]->getColorTexture(0)->release();
		Shading->getColorTexture(0)->release();
	out->disable();
}
void PostProcess::applyDirectionalOcclusion(GPUFBO *GBuffers, GPUFBO *Depth_FBO_Normalized, GPUFBO *out)
{
	//----- Ambient Occlusion -----//
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DO_PRG_DO);

	doBaseRadius->Set( doBaseRadiusVal );
	doMaxOcclusionDist->Set( doMaxOcclusionDistVal);

	DO_FBO_DO->enable();
	//out->enable();
		GBuffers->getColorTexture(0)->bind(DO_Sampler_DO_GBuffers->getValue());
		Depth_FBO_Normalized->getColorTexture(0)->bind(DO_Sampler_DO_Depth->getValue());
		bindNoiseTexture(DO_Sampler_DO_Noise->getValue());
			this->draw();
		releaseNoiseTexture();
		Depth_FBO_Normalized->getColorTexture(0)->release();
		GBuffers->getColorTexture(0)->release();
	DO_FBO_DO->disable();
	//out->disable();

	//----- Blur Pass -----//
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DO_PRG_DO_HBlur);

	//AO_FBO_AO_Blur[0]->enable();
	DO_FBO_DO_Blur[0]->enable();
		GBuffers->getColorTexture( 0 )->bind( DO_Sampler_DO_HBLUR_GBuffers->getValue() );
		DO_FBO_DO->getColorTexture( 0 )->bind( DO_Sampler_DO_HBlur_DO->getValue() );
			this->draw();
		DO_FBO_DO->getColorTexture(0)->release();
		GBuffers->getColorTexture( 0 )->release();
		DO_FBO_DO_Blur[0]->disable();

	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DO_PRG_DO_VBlur);

	out->enable();
		GBuffers->getColorTexture( 0 )->bind( DO_Sampler_DO_VBLUR_GBuffers->getValue() );
		DO_FBO_DO_Blur[0]->getColorTexture( 0 )->bind( DO_Sampler_DO_VBlur_DO->getValue() );
			this->draw();
		DO_FBO_DO_Blur[0]->getColorTexture(0)->release();
		GBuffers->getColorTexture( 0 )->release();
	out->disable();
}
void PostProcess::applyToneMapping(GPUFBO *HDRShading, GPUFBO *out)
{
	tmTime = omp_get_wtime();

	if ((tmTime - tmLastTime) >= tmWaitTime)
	{
		tmLastTime = tmTime;

		//----- Luminance Pass -----//
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, TM_PRG_Luminance_Computation);

		TM_FBO_Luminance->enable();
			HDRShading->getColorTexture(0)->bind(TM_Sampler_Luminance_Computation_HDRLuminance->getValue());
				this->draw();
			HDRShading->getColorTexture(0)->release();
		TM_FBO_Luminance->disable();

		//----- Downscale Pass -----//
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, TM_PRG_Luminance_Downscale);

		tmLuminanceHStep->Set(1.0f / TM_FBO_Luminance_Downscale[0]->getHeight());
		tmLuminanceVStep->Set(1.0f / TM_FBO_Luminance_Downscale[0]->getWidth());
		TM_FBO_Luminance_Downscale[0]->enable();
			TM_FBO_Luminance->getColorTexture(0)->bind(TM_Sampler_Luminance_Downscale_Luminance->getValue());
				this->draw();
			TM_FBO_Luminance->getColorTexture(0)->release();
		TM_FBO_Luminance_Downscale[0]->disable();

		for(unsigned int i = 1; i < tmNumDownscaleBuffers; ++i)
		{
			tmLuminanceHStep->Set(1.0f / TM_FBO_Luminance_Downscale[i]->getHeight());
			tmLuminanceVStep->Set(1.0f / TM_FBO_Luminance_Downscale[i]->getWidth());
			TM_FBO_Luminance_Downscale[i]->enable();
				TM_FBO_Luminance_Downscale[i - 1]->getColorTexture(0)->bind(TM_Sampler_Luminance_Downscale_Luminance->getValue());
					this->draw();
				TM_FBO_Luminance_Downscale[i - 1]->getColorTexture(0)->release();
			TM_FBO_Luminance_Downscale[i]->disable();
		}

		glBindTexture(GL_TEXTURE_2D, TM_FBO_Luminance_Downscale[tmNumDownscaleBuffers - 1]->getColorTexture()->getId());
		glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_FLOAT, data);
		glBindTexture(GL_TEXTURE_2D, 0);

		float maxLuminance = 0.0f;


		for (int p = 0; p < (TM_FBO_Luminance_Downscale[tmNumDownscaleBuffers - 1]->getHeight() * TM_FBO_Luminance_Downscale[tmNumDownscaleBuffers - 1]->getWidth()); ++p)
		{
			maxLuminance = (glm::max)(maxLuminance, data[p]);
		}

		//if (maxLuminance < 1.0)
		//	maxLuminance = 1.0;

		const float delta = tmLastMaxLuminance - maxLuminance;

		const float valeur1 = 0.00004; // 0.04
		const float valeur2 = 0.0016; // 0.0016

		if (delta < 0.0f)
		{
			if (tmNewPower > valeur1)
			{
				tmLastPower += valeur1;
				tmNewPower -= valeur1;
			}

			if (tmWaitTime > valeur1)
				tmWaitTime -= valeur2;
		}
		else
		{
			if (tmLastPower > valeur1)
			{
				tmLastPower -= valeur1;
				tmNewPower += valeur1;
			}
			tmWaitTime += valeur2;
		}

		tmLastMaxLuminance = (tmLastMaxLuminance * tmLastPower) + (maxLuminance * tmNewPower);

		if (tmLastMaxLuminance < 1.0f)
			tmLastMaxLuminance = 1.0f;
	}
	
	//----- ToneMapping Pass -----//
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, TM_PRG_ToneMapping);

	tmMaxRGBValue->Set(tmLastMaxLuminance);
	out->enable();
		HDRShading->getColorTexture(0)->bind(TM_Sampler_ToneMapping_HDR->getValue());
			this->draw();
		HDRShading->getColorTexture(0)->release();
	out->disable();
}
void PostProcess::applyBloom(GPUFBO *Shading, GPUFBO *out)
{
	//----- Bright Pass -----//
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, Bl_PRG_Bright_Computation);

	Bl_FBO_Bright->enable();
		Shading->getColorTexture(0)->bind(Bl_Sampler_Bright_Computation_Color->getValue());
			this->draw();
		Shading->getColorTexture(0)->release();
	Bl_FBO_Bright->disable();

	//----- Blur Pass -----//
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, Bl_PRG_Bright_HBlur);

	Bl_FBO_Bright_Blur[0]->enable();
		Bl_FBO_Bright->getColorTexture(0)->bind(Bl_Sampler_Bright_HBlur_Bright->getValue());
			this->draw();
		Bl_FBO_Bright->getColorTexture(0)->release();
	Bl_FBO_Bright_Blur[0]->disable();

	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, Bl_PRG_Bright_VBlur);

	Bl_FBO_Bright_Blur[1]->enable();
		Bl_FBO_Bright_Blur[0]->getColorTexture(0)->bind(Bl_Sampler_Bright_VBlur_Bright->getValue());
			this->draw();
		Bl_FBO_Bright_Blur[0]->getColorTexture(0)->release();
	Bl_FBO_Bright_Blur[1]->disable();

	//----- Bloom Pass -----//
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, Bl_PRG_Bloom);

	out->enable();
		Shading->getColorTexture(0)->bind(Bl_Sampler_Bloom_Color->getValue());
		Bl_FBO_Bright_Blur[1]->getColorTexture(0)->bind(Bl_Sampler_Bloom_Blur->getValue());
			this->draw();
		Bl_FBO_Bright_Blur[1]->getColorTexture(0)->release();
		Shading->getColorTexture(0)->release();
	out->disable();
}
void PostProcess::applyDepthOfField(GPUFBO *Depth_FBO_Normalized, GPUFBO *Shading, GPUFBO *out)
{
	//----- Focal Depth Pass -----//
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DOF_PRG_Focal_Depth);

	DOF_FBO_Focal->enable();
		Depth_FBO_Normalized->getColorTexture(0)->bind(DOF_Sampler_Focal_Depth_Depth->getValue());
			this->draw();
		Depth_FBO_Normalized->getColorTexture(0)->release();
	DOF_FBO_Focal->disable();

	//----- Focal Blur Pass -----//
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DOF_PRG_Focal_HBlur);

	DOF_FBO_Focal_Blur[0]->enable();
		DOF_FBO_Focal->getColorTexture(0)->bind(DOF_Sampler_Focal_HBlur_Focal->getValue());
			this->draw();
		DOF_FBO_Focal->getColorTexture(0)->release();
	DOF_FBO_Focal_Blur[0]->disable();

	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DOF_PRG_Focal_VBlur);

	DOF_FBO_Focal_Blur[1]->enable();
		DOF_FBO_Focal_Blur[0]->getColorTexture(0)->bind(DOF_Sampler_Focal_VBlur_Focal->getValue());
			this->draw();
		DOF_FBO_Focal_Blur[0]->getColorTexture(0)->release();
	DOF_FBO_Focal_Blur[1]->disable();

	//----- Shading Blur Pass -----//
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DOF_PRG_Shading_HBlur);

	DOF_FBO_Shading_Blur[0]->enable();
		Shading->getColorTexture(0)->bind(DOF_Sampler_Shading_HBlur_Shading->getValue());
		DOF_FBO_Focal_Blur[1]->getColorTexture(0)->bind(DOF_Sampler_Shading_HBlur_Focal->getValue());
			this->draw();
		DOF_FBO_Focal_Blur[1]->getColorTexture(0)->release();
		Shading->getColorTexture(0)->release();
	DOF_FBO_Shading_Blur[0]->disable();

	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DOF_PRG_Shading_VBlur);

	DOF_FBO_Shading_Blur[1]->enable();
		DOF_FBO_Shading_Blur[0]->getColorTexture(0)->bind(DOF_Sampler_Shading_VBlur_Shading->getValue());
		DOF_FBO_Focal_Blur[1]->getColorTexture(0)->bind(DOF_Sampler_Shading_VBlur_Focal->getValue());
			this->draw();
		DOF_FBO_Focal_Blur[1]->getColorTexture(0)->release();
		DOF_FBO_Shading_Blur[0]->getColorTexture(0)->release();
	DOF_FBO_Shading_Blur[1]->disable();

	//----- Depth of Field -----//
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, DOF_PRG_DepthOfField);

	out->enable();
		Shading->getColorTexture(0)->bind(DOF_Sampler_DepthOfField_Color->getValue());
		DOF_FBO_Focal_Blur[1]->getColorTexture(0)->bind(DOF_Sampler_DepthOfField_Focal->getValue());
		DOF_FBO_Shading_Blur[1]->getColorTexture(0)->bind(DOF_Sampler_DepthOfField_Blur->getValue());
			this->draw();
		DOF_FBO_Shading_Blur[1]->getColorTexture(0)->release();
		DOF_FBO_Focal_Blur[1]->getColorTexture(0)->release();
		Shading->getColorTexture(0)->release();
	out->disable();
}
void PostProcess::applyFog(GPUFBO *AA_FBO_Edge, GPUFBO *Shading, GPUFBO *out)
{
	//----- Fog Pass -----//
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, FOG_PRG_Fog);

	out->enable();
		Shading->getColorTexture(0)->bind(FOG_Sampler_Fog_Color->getValue());
		AA_FBO_Edge->getColorTexture(0)->bind(FOG_Sampler_Fog_Depth->getValue());
			this->draw();
		AA_FBO_Edge->getColorTexture(0)->release();
		Shading->getColorTexture(0)->release();
	out->disable();
}

void PostProcess::applyMotionBlur (GPUFBO *GBuffers, GPUFBO *Shading, GPUFBO *out) {
	auto now = std::chrono::high_resolution_clock::now();
	float delta = std::chrono::duration_cast< std::chrono::duration<long, std::milli > > (now - lastUpdateTime).count () * 0.001;
	lastUpdateTime = now;

	float FPS = 1 / ( delta );
	float lastFrameFPS = 1 / ( lastDelta );
	lastDelta = delta;

	float timeForMB = 0.9 * lastFrameFPS + 0.1 * FPS;

	velocityScale->Set (timeForMB*0.01666); // 0.016 = 1 / 60 => FPS Target

	glDisable (GL_DEPTH_TEST);
	m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, MB_PRG_MotionBlur);
	
	out->enable ();
	Shading->getColorTexture (0)->bind (MB_Sampler_MotionBlur_Color->getValue ());
	GBuffers->getColorTexture (0)->bind (MB_Sampler_MotionBlur_Velocity->getValue ());
	this->draw ();
	Shading->getColorTexture (0)->release ();
	GBuffers->getColorTexture (0)->release ();
	out->disable ();

}


void PostProcess::applyTest (GPUFBO *Depth, GPUFBO *Shading, GPUFBO *out) {
	testTime->Set (omp_get_wtime ());
	Camera* camera = Scene::getInstance ()->camera ();
	m_ProgramPipeline->useProgramStage (GL_FRAGMENT_SHADER_BIT, TEST_PRG_Test);
	out->enable ();
	Shading->getColorTexture (0)->bind (TEST_Sampler_Test_Color->getValue ());
	Depth->getColorTexture (0)->bind (TEST_Sampler_Test_Depth->getValue ());
	this->draw ();
	Shading->getColorTexture (0)->release ();
	Depth->getColorTexture (0)->release ();
	out->disable ();
}