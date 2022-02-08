#include "Engine/OpenGL/LightingModelGL.h"
#include "Engine/OpenGL/Managers/GLUniformManager.h"
#include "Engine/Base/Scene.h"




LightBuffer::LightBuffer(int lightsNumber){
    this->LightsArray = new Light[lightsNumber];
}


void printLightBuffer(LightBuffer* p)
{
    
	std::cout << "camPos = (" << p->data.camPos.x << ";" << p->data.camPos.y << ";" << p->data.camPos.z << ";" << p->data.camPos.w << ")\n";
	std::cout << "nbLights = (" << p->data.nbLights.x << ";" << p->data.nbLights.y << ";" << p->data.nbLights.z << ";" << p->data.nbLights.w << ")\n";
	for (int i = 0; i < p->data.nbLights.x; i++)
	{
		std::cout << "Light=[" << i << "] = { \n ";
		printLight(&(p->LightsArray[i]));
		std::cout << "}\n";
	}
}


LightingModelGL::LightingModelGL(std::string name):
	LightingModel(name)
{
    // Default : 1 light
    lightParams = new LightBuffer();
	m_Buffer = new GPUBuffer("LightingBuffer");
    m_Buffer->create((int) (sizeof(LightBuffer::GPUData) + sizeof(Light)), GL_UNIFORM_BUFFER, GL_DYNAMIC_COPY);
    m_Buffer->setBindingPoint(LIGHTING_UNIFORM_BINDING);
    
    GLUniformManager::storeInSharedBuffersList(m_Buffer);

}

LightingModelGL::LightingModelGL(std::string name, Node* root)
	:LightingModel(name)
{
	this->collect(root);
    
    lightParams = new LightBuffer((int)collector->nodes.size());
	m_Buffer = new GPUBuffer("LightingBuffer");
    m_Buffer->create((int) (sizeof(LightBuffer::GPUData) + (int)collector->nodes.size() * sizeof(Light)), GL_UNIFORM_BUFFER, GL_DYNAMIC_COPY);
    m_Buffer->setBindingPoint(LIGHTING_UNIFORM_BINDING);
    GLUniformManager::storeInSharedBuffersList(m_Buffer);
	
	//std::cout << "Monitoring size of elements :" << std::endl;
	//std::cout << "lightParams Theorical size = " << (sizeof(LightBuffer)+collector->nodes.size() * sizeof(Light)) << std::endl;
	//std::cout << "NB lights = " << collector->nodes.size() << std::endl;
	//std::cout << "Sizeof light = " << sizeof(Light) << std::endl;
	//std::cout << "Sizeof vec4 = " << sizeof(glm::vec4) << std::endl;
	//std::cout << "Sizeof ivec4 = " << sizeof(glm::ivec4) << std::endl;
	//int sizeOfLightParams = sizeof(Light)*collector->nodes.size() + sizeof(glm::vec4) + sizeof(glm::ivec4);
	//std::cout << "Test lightbuffer : " << sizeof((*lightParams)) << " vs theorical lightbuffer : " << sizeOfLightParams << std::endl;

}

LightingModelGL::~LightingModelGL()
{
	delete m_Buffer;
}
GPUBuffer* LightingModelGL::getBuffer()
{
	return m_Buffer;
}

void LightingModelGL::setWindowSize(glm::vec2 size)
{
	wSize = size;
	needUpdate = true;
}

void LightingModelGL::update(bool forceUpdate)
{
	LightingModel::update();
	needUpdate =  needUpdate || forceUpdate;

	bool camNeedUpdate = Scene::getInstance()->camera()->needUpdate();
	if (camNeedUpdate)
	{
		lightParams->data.camPos = glm::vec4(Scene::getInstance()->camera()->convertPtTo(glm::vec3(0.0, 0.0, 0.0), Scene::getInstance()->getRoot()->frame()), 1.0);
	}
	if (needUpdate)
	{
		lightParams->data.nbLights = glm::ivec4((int)collector->nodes.size(), 0, 0, 0);

		// iterators
		int i = 0;
		for (vector< Node* >::iterator it = collector->nodes.begin(); it != collector->nodes.end(); ++it)
		{
			LightNode *lnode = dynamic_cast< LightNode* > ((*it));
            if(lnode != nullptr){
                lightParams->LightsArray[i].color = lnode->getParams().color;
                lightParams->LightsArray[i].direction = lnode->getParams().direction;
                lightParams->LightsArray[i].info = lnode->getParams().info;
                lightParams->LightsArray[i].position = lnode->getParams().position;
            }else{
                std::cout<< "Warning : lnode detect to nullptr when updating light buffer "<<std::endl;
            }
            
			i++;
		}
	}
	if (needUpdate || camNeedUpdate)
	{
		
		m_Buffer->bind();
		GPULightBuffer* ptr = (GPULightBuffer*)m_Buffer->map(GL_WRITE_ONLY);
		if (ptr != NULL)
		{
            ptr->campPos = lightParams->data.camPos;
            ptr->nbLights = lightParams->data.nbLights;
            for(int i = 0; i < ptr->nbLights.x; ++i)
            {
                ptr->Lights[i] = lightParams->LightsArray[i];
            }
            m_Buffer->unMap();
		}
		else std::cout << "Failed to update lightbuffer" << std::endl;
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
		needUpdate = false;

	}
	m_Buffer->bind();
	

}

void LightingModelGL::bind(int location)
{
	m_Buffer->bind(location);
}

void LightingModelGL::renderLights()
{
	for (unsigned int i = 0; i < collector->nodes.size(); i++) 
		collector->nodes[i]->render();
}