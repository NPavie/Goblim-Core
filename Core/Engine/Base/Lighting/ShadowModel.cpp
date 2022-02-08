#include "ShadowModel.h"
#include "Engine/Base/Node.h"
#include "Engine/Base/Scene.h"


typedef struct remote_node{
	glm::vec4 plane;
	glm::uvec4 link;	
}r_node;

ShadowModel::ShadowModel(bool c,Node * n)
{
	m_cast 	= c;
	if(c && n->getModel()!=NULL)
		{
			m_BufferMaxSize = 3*n->getModel()->getGeometricModel()->nb_faces;
			initSSBO();
		} 
		else m_BufferMaxSize = 0;
	inited = false;
	dad= n;
}

ShadowModel::~ShadowModel()		{}

bool ShadowModel::castShadow()				{ return m_cast;}

void ShadowModel::setCastShadow(bool b)
{
	m_cast = b;
	m_BufferMaxSize = dad->getModel()->getGeometricModel()->nb_faces;
	if (m_cast) initSSBO();
}

GPUBuffer * ShadowModel::getLsp2SSBO()		{ return m_nodes; }

GPUBuffer * ShadowModel::getRoots()			{ return m_roots; }

void ShadowModel::initSSBO()
{
	if (m_BufferMaxSize!=0 && !inited)
	{	
		m_nodes = new GPUBuffer("Nodes");
		if (m_nodes!=0)	
			LOG(TRACE) <<"SHADOW MODEL:Nodes SSBO created size : "<<4*m_BufferMaxSize;
		m_nodes->create( (1+m_BufferMaxSize) * 4 * sizeof(remote_node), GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_COPY);

		m_roots = new GPUBuffer("Roots");
		m_roots->create( 8*sizeof(GLuint), GL_SHADER_STORAGE_BUFFER ,GL_DYNAMIC_COPY);
		inited = true;
		glFinish();
	}
}
