#include "PBRMaterial.hpp"


PBRMaterial::PBRMaterial (std::string name, GPUTextureCubeMap* cube) :
MaterialGL (name, "PBRMaterial")
{
	modelViewProj = vp->uniforms ()->getGPUmat4 ("MVP");
	M = vp->uniforms ()->getGPUmat4 ("M");

	normalM = vp->uniforms ()->getGPUmat4 ("NormalM");

	roughness = fp->uniforms ()->getGPUfloat ("roughness");
	roughness->Set (0.36f);
	F0 = fp->uniforms ()->getGPUvec3 ("F0");
	F0->Set (glm::vec3(0.04f));

	albedo = fp->uniforms ()->getGPUvec3 ("albedo");
	albedo->Set (glm::vec3 (0.5f));

	camPos = fp->uniforms ()->getGPUvec3 ("CamPos");

	fp_hasCubeMap = fp->uniforms ()->getGPUbool ("hasCubeMap");

	if (cube != NULL) {
		cubeMap = cube;
		hasCubeMap = true;
		fp_hasCubeMap->Set (true);
		fp_cubeMap = fp->uniforms ()->getGPUsampler ("cubeMap");
		fp_cubeMap->Set (0);
	}
	else 
		fp_hasCubeMap->Set (false);
}
PBRMaterial::PBRMaterial (std::string name, glm::vec3 base, float smoothness, float metal, float reflectance) :
MaterialGL (name, "PBRMaterial"){
	modelViewProj = vp->uniforms ()->getGPUmat4 ("MVP");
	M = vp->uniforms ()->getGPUmat4 ("M");

	normalM = vp->uniforms ()->getGPUmat4 ("NormalM");


	roughness = fp->uniforms ()->getGPUfloat ("roughness");
	roughness->Set (1 - smoothness*smoothness);
	F0 = fp->uniforms ()->getGPUvec3 ("F0");
	F0->Set (0.16f*(reflectance*reflectance)*(1.0f - metal) + base*metal);

	albedo = fp->uniforms ()->getGPUvec3 ("albedo");
	albedo->Set (base);

	metalMask = metal;
	reflectance = reflectance;

	camPos = fp->uniforms ()->getGPUvec3 ("CamPos");

	fp_hasCubeMap = fp->uniforms ()->getGPUbool ("hasCubeMap");
	fp_hasCubeMap->Set (false);
}

PBRMaterial::~PBRMaterial ()
{

}

void PBRMaterial::setRoughness (float s) {
	smoothness = s;
	roughness->Set (1 - smoothness*smoothness);
}

void PBRMaterial::setF0 (glm::vec3 f) {
	F0->Set (f);
}

void PBRMaterial::setAlbedo (glm::vec3 a) {
	albedo->Set (a);
}

void PBRMaterial::render (Node *o)
{
	if (m_ProgramPipeline)
	{
		if (hasCubeMap)
			cubeMap->bind (fp_cubeMap->getValue ());

		m_ProgramPipeline->bind ();
		o->drawGeometry (GL_TRIANGLES);
		m_ProgramPipeline->release ();
	}
}

void PBRMaterial::update (Node* o, const int elapsedTime)
{
	if (o->frame ()->updateNeeded ())
	{
		glm::mat4 modelV = Scene::getInstance ()->camera ()->getModelViewMatrix (o->frame ());
		normalM->Set (glm::transpose (glm::inverse (o->frame ()->getRootMatrix ())));
		M->Set (o->frame ()->getRootMatrix ());
		modelViewProj->Set (o->frame ()->getTransformMatrix ());
		camPos->Set (Scene::getInstance ()->camera ()->convertPtTo (glm::vec3 (0.0), Scene::getInstance ()->getRoot ()->frame ()));
		o->frame ()->setUpdate (false);
	}
	else 	if (Scene::getInstance ()->camera ()->needUpdate ())
	{
		glm::mat4 modelV = Scene::getInstance ()->camera ()->getModelViewMatrix (o->frame ());
		normalM->Set (glm::transpose (glm::inverse (o->frame ()->getRootMatrix ())));
		M->Set (o->frame ()->getRootMatrix ());
		modelViewProj->Set (o->frame ()->getTransformMatrix ());
		camPos->Set (Scene::getInstance ()->camera ()->convertPtTo (glm::vec3 (0.0), Scene::getInstance ()->getRoot ()->frame ()));
		modelViewProj->Set (o->frame ()->getTransformMatrix ());
	}


}