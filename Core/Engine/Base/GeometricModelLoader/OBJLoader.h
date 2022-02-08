#include "Engine/Base/GeometricModelLoader.h"
using namespace std;
class GeometricModel;

class OBJLoader : public GeometricModelLoader
{
	public:
		OBJLoader();
		~OBJLoader();
		virtual bool loadModel(string name,GeometricModel *model);
	private:
		void setupForTextureCoordinates(GeometricModel* model);
		void computeNormals(GeometricModel* model);
		void computeTangents(GeometricModel* model);
};
