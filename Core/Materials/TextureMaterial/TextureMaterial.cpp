#include "TextureMaterial.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Engine.h"


TextureMaterial::TextureMaterial(std::string name, GPUTexture *color, GPUTexture *normal, LightingModelGL *lightingM) :
MaterialGL(name, "TextureMaterial")
{
	normalTex = NULL;
	modelViewProj = vp->uniforms()->getGPUmat4("MVP");
	modelView = vp->uniforms()->getGPUmat4("MV");
	colorSampler = fp->uniforms()->getGPUsampler("colorTex");
	normalSampler = fp->uniforms()->getGPUsampler("normalTex");

	use_normal = fp->uniforms()->getGPUbool("use_normal");


	modelViewF = fp->uniforms()->getGPUmat4("NormalMV");
	modelMatrix = fp->uniforms()->getGPUmat4("modelMatrix");

	//colorSampler->Set(color->getHandle());
	//if (normal != NULL)
	//	normalSampler->Set(normal->getHandle());

	colorSampler->Set(0);
	setColorMap(color);

	if (normal != NULL)
	{
		
		((GPUTexture2D*)normal)->setUpSampler(GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		setNormalMap(normal);
		normalSampler->Set(1);
	}
	lighting = lightingM;
	

	if (lighting != NULL)
		fp->uniforms()->mapBufferToBlock(lighting->getBuffer(), "LightingUBO");
	

	val = 0;
		


}
TextureMaterial::~TextureMaterial()
{

}
void TextureMaterial::setColorMap(GPUTexture *color)
{
	colorTex = color;
}
void TextureMaterial::setNormalMap(GPUTexture *normal)
{
	normalTex = normal;
	useNormal(true);
}
void TextureMaterial::useNormal(bool b)
{
	use_normal->Set(b && normalTex != NULL);
}
void TextureMaterial::render(Node *o)
{
	if (m_ProgramPipeline)
	{

		colorTex->bind(0);
		if (normalTex != NULL)	
			normalTex->bind(normalSampler->getValue());


		m_ProgramPipeline->bind();
		o->drawGeometry(GL_TRIANGLES);
		m_ProgramPipeline->release();

		colorTex->release();

		if (normalTex != NULL)
			normalTex->release();

	}
}

void TextureMaterial::update(Node* o, const int elapsedTime)
{
	if (o->frame()->updateNeeded())
	{
		modelView->Set(o->frame()->getRootMatrix());
		modelViewProj->Set(o->frame()->getTransformMatrix());
		modelViewF->Set(glm::transpose(glm::inverse(o->frame()->getRootMatrix())));
		modelMatrix->Set(o->frame()->getRootMatrix());
		o->frame()->setUpdate(false);
	}
	else 	if (Scene::getInstance()->camera()->needUpdate())
	{
		modelViewProj->Set(o->frame()->getTransformMatrix());

	}


}

void TextureMaterial::changeRes()
{

	val = (val + 1) % 3;
	valGpu->Set(val);
	LOG(INFO)  << "changing res to " << val ;

}