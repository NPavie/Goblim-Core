#include "Model.h"

Resource_mgr<GeometricModel> Model::m_ModelMgr = Resource_mgr<GeometricModel>();

Model::Model(std::string name,bool loadnow)
{
	m_Name = name;
	
	try
	{
		if (loadnow)
			m_Model = m_ModelMgr.get(name);
		else
		{
			m_Model = new GeometricModel();
			m_ModelMgr.insert(name,m_Model);
		}
	}
	catch(const std::exception & e )
	{
		delete m_Model;
		m_Model = NULL;
		throw logic_error(string("ERROR : Model ") + string(" : \n") + e.what());
	}

}

Model::~Model()
{
	m_ModelMgr.release(m_Name);
}
GeometricModel* Model::getGeometricModel()
{
	return m_Model;
}
void Model::drawGeometry(int type)
{
}
std::string Model::name()
{
	return m_Name;
}