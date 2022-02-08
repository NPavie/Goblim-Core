#include "Materials/DeferredMaterial/DeferredMaterial.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Engine.h"


DeferredMaterial::DeferredMaterial (std::string name, 
	GPUTexture *color, 
	GPUTexture *ao, 
	GPUTexture *metalMask, 
	GPUTexture *roughness, 
	GPUTexture *reflectance, 
	GPUTexture *normal, 
	float bumpScale, 
	float maxQuality, 
	bool height, 
	bool ss, 
	glm::vec3 metal_reflectance_roughness,
	glm::vec3 baseColor,
	LightingModelGL *lightingM): 
MaterialGL(name, "DeferredMaterial")
{

	GPUTexture *fs_colorTex, *fs_aoTex, *fs_metalMaskTex, *fs_roughnessTex, *fs_reflectanceTex, *fs_normalTex;
	GPUsampler *fs_colorSampler, *fs_aoSampler, *fs_metalMaskSampler, *fs_roughnessSampler, *fs_reflectanceSampler, *fs_normalSampler;

	fs_colorTex = NULL; fs_aoTex = NULL; fs_metalMaskTex = NULL; fs_roughnessTex = NULL; fs_reflectanceTex = NULL; fs_normalTex = NULL;
	
	//----- Vertex Program Parameters ---------------------//
	this->vs_MV = vp->uniforms()->getGPUmat4("MV");
	this->vs_MVP = vp->uniforms ()->getGPUmat4 ("MVP");
	this->vs_LastMVP = vp->uniforms ()->getGPUmat4 ("lastMVP");
	this->vs_NormalMV = vp->uniforms ()->getGPUmat4 ("NormalMV");
	this->vs_NormalM = vp->uniforms ()->getGPUmat4 ("NormalM");
	this->vs_M = vp->uniforms ()->getGPUmat4 ("M");
	//----- Tesselation Control Program Parameters --------//
	//----- Tesselation Evaluation Program Parameters -----//
	//----- Geometry Program Parameters -------------------//
	//this->gs_MV = gp->uniforms()->getGPUmat4("MV");
	//this->gs_MVP = gp->uniforms()->getGPUmat4("MVP");
	//this->gs_NormalMV = gp->uniforms()->getGPUmat4("NormalMV");
	//this->gs_CamPos = gp->uniforms()->getGPUvec3("CamPos");
	//----- Fragment Program Parameters -------------------//
	/*this->fs_M = fp->uniforms()->getGPUmat4("M");
	this->fs_NormalMV = fp->uniforms ()->getGPUmat4 ("NormalMV");
	this->fs_NormalM = fp->uniforms ()->getGPUmat4 ("NormalM");*/
	this->fs_CamPos = fp->uniforms()->getGPUvec3("CamPos");
	this->fs_bumpScale = fp->uniforms()->getGPUfloat("bumpScale");
	this->fs_bumpScale->Set(bumpScale);
	this->fs_maxQuality = fp->uniforms()->getGPUfloat("MaxQuality");
	this->fs_maxQuality->Set(maxQuality);
	

	fs_baseColor = fp->uniforms ()->getGPUvec3 ("base_Color");

	//----- CLASSIC TEXTURE USE ---------------------------//

	// Diffuse Mapping
	fs_colorSampler = fp->uniforms ()->getGPUsampler ("baseColorTex");
	fs_use_base_color = fp->uniforms ()->getGPUbool ("use_base_color");
	if (color != NULL) {
		fs_colorTex = color;
		//fs_colorTex->makeResident ();
		fs_colorSampler->Set (0);
		//fs_colorSampler->Set (fs_colorTex->getHandle ());
		fs_use_base_color->Set (true);
		texturesGPU.push_back (Texture_Sampler (fs_colorTex, fs_colorSampler));
	}
	else {
		fs_use_base_color->Set (false);
		fs_baseColor->Set (baseColor);
	}
	// AO Mapping
	fs_aoSampler = fp->uniforms()->getGPUsampler("aoTex");
	fs_use_ao = fp->uniforms()->getGPUbool("use_ao");
	if (ao != NULL)
	{
		fs_aoTex = ao;
		//fs_aoTex->makeResident();
		//fs_aoSampler->Set(fs_aoTex->getHandle());
		fs_aoSampler->Set (1);
		fs_use_ao->Set(true);
		texturesGPU.push_back (Texture_Sampler (fs_aoTex, fs_aoSampler));
	}
	else
		fs_use_ao->Set(false);

	fs_metalMask_reflectance_roughness = fp->uniforms ()->getGPUvec3 ("metalMask_Reflectance_Roughness");
	fs_metalMask_reflectance_roughness->Set (metal_reflectance_roughness);


	// MetalMask Mapping
	fs_metalMaskSampler = fp->uniforms()->getGPUsampler("metalMaskTex");
	fs_use_metalMask = fp->uniforms()->getGPUbool("use_metal_mask");
	if (metalMask != NULL)
	{
		fs_metalMaskTex = metalMask;
		//fs_metalMaskTex->makeResident ();
		//fs_metalMaskSampler->Set (fs_metalMaskTex->getHandle());
		fs_metalMaskSampler->Set (2);
		fs_use_metalMask->Set (true);
		texturesGPU.push_back (Texture_Sampler (fs_metalMaskTex, fs_metalMaskSampler));
	}
	else
		fs_use_metalMask->Set (false);

	//
	fs_roughnessSampler = fp->uniforms()->getGPUsampler("roughnessTex");
	fs_use_roughness = fp->uniforms ()->getGPUbool ("use_roughness");
	if (roughness != NULL)
	{
		fs_roughnessTex = roughness;
		//fs_roughnessTex->makeResident ();
		//fs_roughnessSampler->Set (fs_roughnessTex->getHandle());
		fs_roughnessSampler->Set (5);
		//fs_use_roughness->Set (true);
		texturesGPU.push_back (Texture_Sampler (fs_roughnessTex, fs_roughnessSampler));
	}
	else
		fs_use_roughness->Set (false);

	//
	fs_reflectanceSampler = fp->uniforms ()->getGPUsampler ("reflectanceTex");
	fs_use_reflectance = fp->uniforms ()->getGPUbool ("use_reflectance");
	if (reflectance != NULL)
	{
		fs_reflectanceTex = reflectance;
		//fs_reflectanceTex->makeResident ();
		//fs_reflectanceSampler->Set (fs_reflectanceTex->getHandle());
		fs_reflectanceSampler->Set (4);
		fs_use_reflectance->Set (true);
		texturesGPU.push_back (Texture_Sampler (fs_reflectanceTex, fs_reflectanceSampler));
	}
	else
		fs_use_reflectance->Set (false);

	// Normal Mapping
	fs_normalSampler = fp->uniforms()->getGPUsampler("normalTex");
	fs_use_normal = fp->uniforms()->getGPUbool("use_normal");
	if (normal != NULL)
	{
		fs_normalTex = normal;
		//fs_normalTex->makeResident ();
		//fs_normalSampler->Set(fs_normalTex->getHandle());
		fs_normalSampler->Set(3);
		fs_use_normal->Set(true);
		texturesGPU.push_back (Texture_Sampler (fs_normalTex, fs_normalSampler));
	}
	else
		fs_use_normal->Set(false);

	// Parallax Mapping
	fs_use_height = fp->uniforms()->getGPUbool("use_height");
	fs_use_height->Set(height);
	fs_use_selfshadow = fp->uniforms()->getGPUbool("use_selfshadow");
	fs_use_selfshadow->Set(ss && height);

	lighting = lightingM;

	if (lighting != NULL)
		fp->uniforms()->mapBufferToBlock(lighting->getBuffer(), "Lighting");
}

DeferredMaterial::~DeferredMaterial(){}

void DeferredMaterial::render(Node *o)
{
	glClearColor (0.0, 0.0, 0.0, 0.0);
	if (m_ProgramPipeline)
	{
		/*if (fs_colorTex != NULL)
			fs_colorTex->bind((int)fs_colorSampler->getValue());
		if (fs_aoTex != NULL)
			fs_aoTex->bind((int)fs_aoSampler->getValue());
		if (fs_metalMaskTex != NULL)
			fs_metalMaskTex->bind ((int)fs_metalMaskSampler->getValue ());
		if (fs_roughnessTex != NULL)
			fs_roughnessTex->bind ((int)fs_roughnessSampler->getValue ());
		if (fs_reflectanceTex != NULL)
			fs_reflectanceTex->bind ((int)fs_reflectanceSampler->getValue ());
		if (fs_normalTex != NULL)
			fs_normalTex->bind((int)fs_normalSampler->getValue());*/

		for (int i = 0; i < texturesGPU.size (); i++) {
			texturesGPU[i].tex->bind ((int)(texturesGPU[i].sampler->getValue ()));
		}

		m_ProgramPipeline->bind();
		o->drawGeometry(GL_TRIANGLES);
		m_ProgramPipeline->release();

		for (int i = 0; i < texturesGPU.size (); i++) {
			texturesGPU[i].tex->release ();
		}

		/*if (fs_colorTex != NULL)
			fs_colorTex->release();
		if (fs_aoTex != NULL)
			fs_aoTex->release();
		if (fs_metalMaskTex != NULL)
			fs_metalMaskTex->release ();
		if (fs_roughnessTex != NULL)
			fs_roughnessTex->release ();
		if (fs_reflectanceTex != NULL)
			fs_reflectanceTex->release ();
		if (fs_normalTex != NULL)
			fs_normalTex->release();*/
	}
}

void DeferredMaterial::update(Node* o, const int elapsedTime)
{

	Scene* scene = Scene::getInstance ();
	glm::mat4 MV = scene->camera ()->getModelViewMatrix (o->frame ());
	glm::mat4 MVP = o->frame()->getTransformMatrix();
	glm::mat4 NormalMV = glm::transpose (glm::inverse (MV));
	glm::mat4 M = o->frame ()->getRootMatrix ();
	glm::mat4 NormalM = glm::transpose (glm::inverse (M));
	vs_LastMVP->Set (vs_MVP->getValue ());

	glm::vec3 cameraPos = scene->camera ()->convertPtTo (glm::vec3 (0.0), scene->getRoot ()->frame ());
	if (o->frame()->updateNeeded())
	{
		vs_M->Set (M);
		vs_MV->Set(MV);
		vs_MVP->Set(MVP);
		vs_NormalMV->Set(NormalMV);
		vs_NormalM->Set (NormalM);

		//gs_MV->Set(MV);
		//gs_MVP->Set(MVP);
		//gs_NormalMV->Set(NormalMV);
		//gs_CamPos->Set (cameraPos);

		fs_CamPos->Set (cameraPos);

		/*fs_M->Set(M);
		fs_NormalMV->Set(NormalMV);
		fs_NormalM->Set (NormalM);*/
		//o->frame()->setUpdate(false);
	}
	else 	if (scene->camera()->needUpdate())
	{
		vs_M->Set (M);
		vs_MV->Set(MV);
		vs_MVP->Set(MVP);
		vs_NormalMV->Set(NormalMV);
		vs_NormalM->Set (NormalM);

		//gs_MV->Set(MV);
		//gs_MVP->Set(MVP);
		//gs_NormalMV->Set (NormalMV);
		//gs_CamPos->Set (cameraPos);

		fs_CamPos->Set (cameraPos);
	/*	fs_M->Set(M);
		fs_NormalM->Set (NormalM);
		fs_NormalMV->Set(NormalMV);*/
	}
}