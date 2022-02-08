#include "Engine/Base/Scene.h"


INITIALIZE_EASYLOGGINGPP

Scene::Scene()
{
	configureLogger();

	LOG(TRACE) << "Creating Scene" ;
	// Get the root Node 
	m_Root = m_Nodes.get("Root");
	m_Frame = m_Root->frame();
	
	// Create a projective default camera with standard parameter and place it in the scene
	CameraGL* camera = m_Cameras.get("DefaultCamera");
	camera->setPerspectiveProjection(45.0f,1.0f,0.01f,200.0f);
	camera->lookAt(glm::vec3(0.0,0.0,0.0),glm::vec3(0.0,10.0,10.0),glm::vec3(0.0,1.0,0.0));
	
	
	current_Camera = camera;
		
	// Link the camera frame to the scene frame
	camera->link(m_Frame);	

	// Create a Scene Node and link it to root node
	m_Scene = m_Nodes.get("Scene");
	m_Root->adopt(m_Scene);
	current_ManipulatedNode = m_Scene;

	




}
Scene::~Scene()
{
	m_Nodes.release("Root");
}
Frame* Scene::frame()
{
	return m_Frame;
}
CameraGL* Scene::camera()
{
	return current_Camera;
}

void Scene::nextCamera()
{
	current_Camera = m_Cameras.nextObject(current_Camera->getName());
	current_Camera->link(m_Frame);
	current_Camera->bind();

	LOG(INFO) << "using camera " << current_Camera->getName() ;
}
void Scene::nextManipulatedNode()
{
	current_ManipulatedNode = m_Nodes.nextObject(current_ManipulatedNode->getName());
	if (current_ManipulatedNode == m_Root)
		current_ManipulatedNode = m_Nodes.nextObject(current_ManipulatedNode->getName());
	
	LOG(INFO) << "manipulating Node " << current_ManipulatedNode->getName() ;
}
void Scene::manipulateNode(std::string name)
{
	Node* c_node =  m_Nodes.find(name);
	if (c_node == NULL)
		LOG(WARNING) << "Error : Node " << name << " does not exists." ;
	else
	{
		current_ManipulatedNode = c_node;
		LOG(INFO) << "Manipulated node is now " << current_ManipulatedNode->getName();
	}
}
Node* Scene::getSceneNode()
{
	return m_Scene;
}
Node* Scene::getManipulatedNode()
{
	return current_ManipulatedNode;
}

void Scene::useCamera(std::string name)
{
	
	CameraGL* cam = m_Cameras.find(name);
	if (cam == NULL)
		LOG(WARNING) << "Error : Camera " << name << " does not exists ";
	else
	{
		current_Camera = cam;
		cam->link(m_Frame);
		cam->bind();
		LOG(INFO) << "Rendering to camera " << cam->getName() ;
	}
}
CameraGL* Scene::getCamera(std::string name)
{
	return (m_Cameras.find(name));
}


Node* Scene::getRoot()
{
	return m_Root;
}
Node* Scene::getNode(std::string name)
{
	return(m_Nodes.get(name));
}
void Scene::releaseNode(Node *n)
{
	m_Nodes.release(n->getName());
}
void Scene::releaseNode(std::string name)
{
	m_Nodes.release(name);
}

void Scene::releaseResource(string a)
 {
	m_Resources.release(a);
 }
void Scene::releaseResource(Resource *r)
{
	m_Resources.release(r->name());
}

void Scene::releaseModel(string a)
{
	m_Models.release(a);
}
void Scene::releaseModel(Model *m)
{
	m_Models.release(m->name());
}


void Scene::configureLogger()
{
	 el::Configurations defaultConf;
	 defaultConf.setToDefault();
	 defaultConf.set(el::Level::Info, 
            el::ConfigurationType::Format, "%msg");
	 defaultConf.set(el::Level::Warning, 
            el::ConfigurationType::Format, "[Warning] %msg");

	 defaultConf.set(el::Level::Trace, 
            el::ConfigurationType::Format, "%datetime %msg");
	 defaultConf.set(el::Level::Trace, 
            el::ConfigurationType::Filename, "./logs/Trace.log");

	  defaultConf.set(el::Level::Trace, 
		  el::ConfigurationType::ToFile,"true");

	  defaultConf.set(el::Level::Trace, 
		  el::ConfigurationType::ToStandardOutput,"false");

	  el::Loggers::reconfigureLogger("default", defaultConf);
}