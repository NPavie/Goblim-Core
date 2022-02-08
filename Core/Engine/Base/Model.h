
#ifndef _MODEL_H
#define _MODEL_H

#include "Engine/Base/GeometricModel.h"
#include "Resource_mgr.hpp"

class Model
{

	public:

		Model(std::string name,bool loadnow = true);
		~Model();
		
		
		GeometricModel* getGeometricModel();
		virtual void drawGeometry(int type=0);

		std::string name();
	protected:
		std::string m_Name;
		GeometricModel *m_Model;

		static Resource_mgr<GeometricModel> m_ModelMgr;

};


#endif
