#include "SurfacicSpotNoise.h"
#include "Engine/Base/Node.h"



SurfacicSpotNoise::SurfacicSpotNoise(std::string name, int nbSpotPerAxis, int distribID, int noiseType) :
MaterialGL(name,"SurfacicSpotNoise")
{
	modelViewMatrix = vp->uniforms()->getGPUmat4("CPU_modelViewMatrix");
	modelMatrix = fp->uniforms()->getGPUmat4("CPU_modelMatrix");
	// Bind bloc of all spots tree buffer 
	int i = 1;
	GPUBuffer::linkBindingPointToProgramStorageBlock(fp, "LightingBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(fp, "gaussianDataBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(fp, "harmonicDataBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(fp, "constantDataBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(fp, "distribDataBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(fp, "spotDataBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(fp, "spotIndexBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(fp, "sizeOfArraysBuffer", ++i);
	GPUBuffer::linkBindingPointToProgramStorageBlock(fp, "weightsDataBuffer", ++i);

	// Adding and testing another fragment shader file named copy-FS.glsl in the effect/Shaders folder
	//copy = new GLProgram(this->m_ClassName + "-copy", GL_FRAGMENT_SHADER);
	//m_ProgramPipeline->useProgramStage(GL_FRAGMENT_SHADER_BIT, copy);
	//m_ProgramPipeline->link();
		
	fp_distribID = fp->uniforms()->getGPUint("distribID");
	fp_distribID->Set(distribID);

	fp_nbSpotPerAxis = fp->uniforms()->getGPUint("nbSpotPerAxis");
	fp_nbSpotPerAxis->Set(nbSpotPerAxis);

	fp_noiseType = fp->uniforms()->getGPUint("noiseType");
	fp_noiseType->Set(noiseType);

	smp_dataField = fp->uniforms()->getGPUsampler("smp_dataField");
	smp_dataField->Set(0);
	tex_dataField = NULL;

	textureScalingFactor = fp->uniforms()->getGPUfloat("textureScalingFactor");

	noiseMenu = TwNewBar("Noise");
	scaling = 8.0;
	TwAddVarRW(noiseMenu, "Scaling", TW_TYPE_FLOAT, &scaling, "min=0.1 step=0.1");
	

}

SurfacicSpotNoise::~SurfacicSpotNoise()
{	

}

void SurfacicSpotNoise::addDataField(GPUTexture2D * dataTexture)
{
	tex_dataField = dataTexture;
}


void SurfacicSpotNoise::render(Node *o)
{	
	if (m_ProgramPipeline)
	{
		
		glClearColor(1.0,1.0,1.0,1.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
		m_ProgramPipeline->bind();
		if (tex_dataField != NULL) tex_dataField->bind(0);
		textureScalingFactor->Set(scaling);
		o->drawGeometry(GL_TRIANGLES);

		if (tex_dataField != NULL) tex_dataField->release();
		m_ProgramPipeline->release();
	}	
}

void SurfacicSpotNoise::update(Node* o, const int elapsedTime)
{
	if (o->frame()->updateNeeded())
	{
		modelViewMatrix->Set(o->frame()->getTransformMatrix());
		modelMatrix->Set(o->frame()->getRootMatrix());
		//o->frame()->setUpdate(false);
	}
	
}