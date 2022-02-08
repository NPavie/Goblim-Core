#include "Effects/GrassComposer/GrassComposer.h"
#include "Engine/OpenGL/EngineGL.h"

GrassComposer::GrassComposer(std::string name, KernelList* kernels, EngineGL* myengineToUse)
	: EffectGL(name,"GrassComposer")
{
	vp = new GLProgram(this->m_ClassName+"-Base",GL_VERTEX_SHADER);
	fp = new GLProgram(this->m_ClassName+"-AddGrass",GL_FRAGMENT_SHADER);

	vp_MVP = vp->uniforms()->getGPUmat4("MVP");

	rayCompute = new ShellRayCompute(name + "-RayComputation");
	precomputedModel = new TextureGenerator(name+"-ModelComputation");
	rayDataFBO = new GPUFBO(name + "-RayDataFBO");
	
	noiseTexture = new GPUTexture2D(ressourceObjPath + "noise.png");
	smp_noiseTexture = fp->uniforms()->getGPUsampler("smp_noiseTexture");
	
	rayDataFBO->create(myengineToUse->getWidth(), myengineToUse->getHeight(), 3, true, GL_RGBA16F, GL_TEXTURE_2D, 1);	// 3 rendu nécessaire : shell front, back et surface
	
	precomputationDone = false;

	// Samplers
	fp_smp_surfaceData = fp->uniforms()->getGPUsampler("smp_surfaceData");
	fp_smp_shellRayFront = fp->uniforms()->getGPUsampler("smp_shellRayFront");
	fp_smp_shellRayBack = fp->uniforms()->getGPUsampler("smp_shellRayBack");
	fp_smp_shellRaySurface = fp->uniforms()->getGPUsampler("smp_shellRaySurface");
	fp_smp_colorFBO = fp->uniforms()->getGPUsampler("smp_colorFBO");
	fp_smp_colorFBODepth = fp->uniforms()->getGPUsampler("smp_colorFBODepth");
	fp_smp_models = fp->uniforms()->getGPUsampler("smp_models");
	fp_smp_densityMaps = fp->uniforms()->getGPUsampler("smp_densityMaps");
	fp_smp_scaleMaps = fp->uniforms()->getGPUsampler("smp_scaleMaps");
	fp_smp_distributionMaps = fp->uniforms()->getGPUsampler("smp_distributionMaps");
	

	// Matrices
	fp_objectToScreen = fp->uniforms()->getGPUmat4("objectToScreen");
	fp_objectToWorld = fp->uniforms()->getGPUmat4("objectToWorld");
	fp_objectToCamera = fp->uniforms()->getGPUmat4("objectToCamera");
	fp_worldToCamera = fp->uniforms()->getGPUmat4("worldToCamera");

	cameraToApplyOn = Scene::getInstance()->camera();
	this->engineToUse = myengineToUse;

	fp_screenInfo = fp->uniforms()->getGPUvec4("screenInfo");

	fp_gridHeight = fp->uniforms()->getGPUfloat("gridHeight");
	
	fp_renderKernels = fp->uniforms()->getGPUbool("renderKernels");
	fp_renderGrid = fp->uniforms()->getGPUbool("renderGrid");
	fp_activeAA = fp->uniforms()->getGPUbool("activeAA");
	fp_activeShadows = fp->uniforms()->getGPUbool("activeShadows");

	timer = fp->uniforms()->getGPUfloat("timer");
	mousePos = fp->uniforms()->getGPUvec2("mousePos");

	fp_activeWind = fp->uniforms()->getGPUbool("activeWind");
	fp_windFactor = fp->uniforms()->getGPUfloat("windFactor");
	fp_windSpeed = fp->uniforms()->getGPUfloat("windSpeed");
	
	fp_activeMouse = fp->uniforms()->getGPUbool("activeMouse");
	fp_mouseFactor = fp->uniforms()->getGPUfloat("mouseFactor");
	fp_mouseRadius = fp->uniforms()->getGPUfloat("mouseRadius");
		
	// Binding points
	int bindingCounter = 0;
	fp_smp_surfaceData->Set(bindingCounter);
	fp_smp_shellRayFront->Set(++bindingCounter);
	fp_smp_shellRayBack->Set(++bindingCounter);
	fp_smp_shellRaySurface->Set(++bindingCounter);
	fp_smp_colorFBO->Set(++bindingCounter);
	fp_smp_colorFBODepth->Set(++bindingCounter);
	fp_smp_models->Set(++bindingCounter);
	fp_smp_densityMaps->Set(++bindingCounter);
	fp_smp_scaleMaps->Set(++bindingCounter);
	fp_smp_distributionMaps->Set(++bindingCounter);
	smp_noiseTexture->Set(++bindingCounter);
	this->kernels = kernels;
	
	
	m_ProgramPipeline->useProgramStage(GL_VERTEX_SHADER_BIT, vp);
	m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, fp);
	m_ProgramPipeline->link();

	displayData = new DisplayLayer("GrassComposer-displayData");
	//m_ProgramPipeline->bind();

	// Par defaut, le compositeur n'a pas de cible
	target = NULL;

	// Binding manuel des blocks sur un point de binding : au cas ou le binding= ne marcherait pas
	GLuint block_index = 0;
	

	block_index = glGetProgramResourceIndex(fp->getProgram(), GL_SHADER_STORAGE_BLOCK, "LightingBuffer");
	if (block_index != GL_INVALID_INDEX)
	{
		std::cout << name << " :: LightingBuffer found in FP at " << block_index << std::endl;
		glShaderStorageBlockBinding(fp->getProgram(), block_index, LIGHTING_SSBO_BINDING);
	}

	

	block_index = glGetProgramResourceIndex(fp->getProgram(), GL_SHADER_STORAGE_BLOCK, "KernelBuffer");
	if (block_index != GL_INVALID_INDEX)
	{
		std::cout << name << " :: KernelBuffer found in FP at " << block_index << std::endl;
		glShaderStorageBlockBinding(fp->getProgram(), block_index, 3);
	}
	

}

void GrassComposer::apply(GPUFBO* colorFBO)
{
	// 1st pass : transformation du model en textures
	if(precomputationDone == false && target != NULL)
	{
		precomputedModel->render(target);
		
		precomputationDone = true;
	}
	// pass normal
	if(m_ProgramPipeline && target != NULL)
	{
		
		
		gridHeight = kernels->getGridHeight();
		// Calcul des données d'entrées / sorties -> dataFBO
		rayCompute->setShellHeight(gridHeight);
		rayDataFBO->enable();
		rayDataFBO->drawBuffer(0);
		rayCompute->renderShellFront(target);
		rayDataFBO->drawBuffer(1);
		rayCompute->renderShellBack(target);
		rayDataFBO->drawBuffer(2);
		rayCompute->renderSurface(target);
		rayDataFBO->disable();

		fp_gridHeight->Set(gridHeight);
		fp_objectToScreen->Set(target->frame()->getTransformMatrix());
		fp_objectToWorld->Set(target->frame()->getRootMatrix());
		fp_worldToCamera->Set(*(Scene::getInstance()->camera()->getFrame()->getMatrix()));
		
		// temps en seconde en float

		vp_MVP->Set(target->frame()->getTransformMatrix());

		cameraToApplyOn = Scene::getInstance()->camera();
		
		// Probleme d'envoi des données de l'engine vers le shader (TCL)
		fp_screenInfo->Set(glm::vec4(engineToUse->getWidth(), engineToUse->getHeight(), cameraToApplyOn->getZnear(), cameraToApplyOn->getZfar()));
		fp_renderKernels->Set(kernels->renderKernels);
		fp_renderGrid->Set(kernels->renderGrid);

		fp_activeAA->Set(kernels->activeAA);
		fp_activeShadows->Set(kernels->activeShadows);

		fp_activeWind->Set(kernels->activeWind);
		fp_windFactor->Set(kernels->windFactor);
		fp_windSpeed->Set(kernels->windSpeed);
		
		fp_activeMouse->Set(kernels->activeMouse);
		fp_mouseFactor->Set(kernels->mouseFactor);
		fp_mouseRadius->Set(kernels->mouseRadius);

		fp_objectToCamera->Set(cameraToApplyOn->getModelViewMatrix(target->frame()));



		// bindings des textures
		precomputedModel->bindTextureArray(fp_smp_surfaceData->getValue());
		rayDataFBO->bindColorTexture(fp_smp_shellRayFront->getValue(), 0);
		rayDataFBO->bindColorTexture(fp_smp_shellRayBack->getValue(), 1);
		rayDataFBO->bindColorTexture(fp_smp_shellRaySurface->getValue(), 2);
		colorFBO->bindColorTexture(fp_smp_colorFBO->getValue(),0);
		colorFBO->bindColorTexture(fp_smp_colorFBODepth->getValue(),1);
		kernels->bindModels(fp_smp_models->getValue());
		kernels->bindDensityMaps(fp_smp_densityMaps->getValue());
		kernels->bindScaleMaps(fp_smp_scaleMaps->getValue());
		kernels->bindDistributionMaps(fp_smp_distributionMaps->getValue());
		noiseTexture->bind(smp_noiseTexture->getValue());
				
		// -- Draw call
		m_ProgramPipeline->bind();
		quad->drawGeometry(GL_TRIANGLES);
		m_ProgramPipeline->release();
		// */

		// Releasing the data
		noiseTexture->release();
		kernels->releaseTextures();
		colorFBO->releaseColorTexture();
		rayDataFBO->releaseColorTexture();
		precomputedModel->releaseTextureArray();
		

		// Display data test
		//rayDataFBO->display(glm::vec4(0, 0, 0.2, 0.2), 0);
		//rayDataFBO->display(glm::vec4(0, 0.2, 0.2, 0.2), 1);
		//rayDataFBO->display(glm::vec4(0, 0.4, 0.2, 0.2), 2);
		//precomputedModel->getFBO()->display(glm::vec4(0, 0, 0.1, 0.1), 0);


		
	}

}


void GrassComposer::loadMenu()
{


}

void GrassComposer::reload()
{
	try{
		m_ProgramPipeline->release();

		vp = new GLProgram(this->m_ClassName + "-Base", GL_VERTEX_SHADER);
		fp = new GLProgram(this->m_ClassName + "-AddGrass", GL_FRAGMENT_SHADER);

		m_ProgramPipeline->useProgramStage(GL_VERTEX_SHADER_BIT, vp);
		m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, fp);
		m_ProgramPipeline->link();
		
	}
	catch (const exception & e)
	{
		throw e;
	}
}

void GrassComposer::setComposerFor(Node* target)
{
	
	this->target = target;
	
}