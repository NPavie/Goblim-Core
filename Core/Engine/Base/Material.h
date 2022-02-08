#ifndef _MATERIAL_
#define _MATERIAL_
#include <string>

class Node;
class Material
{
	public:
		Material();
		Material(std::string name,std::string className = "Unknown");
		~Material();

		virtual void render(Node *o);
		virtual void update(Node *o,const int elapsedTime);

		const std::string getName();


		bool valid;

	protected:
		std::string m_Name;
		std::string m_ClassName;

};

/*
// Material manager
class MaterialManager : public Singleton<MaterialManager>{
	friend class Singleton<MaterialManager>;
{
public:
	static template <typename T> T* Get(T* ptr,std::string className);

	static template <typename T>
private:
	MaterialManager();
	~MaterialManager();

}


*/



#endif
