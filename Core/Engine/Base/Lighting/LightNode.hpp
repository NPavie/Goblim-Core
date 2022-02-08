#ifndef _GOBLIM_LIGHTNODE_
#define _GOBLIM_LIGHTNODE_

#include <glm/glm.hpp>

#include <string>
#include "Engine/Base/Node.h"
#include "Engine/Base/Camera.h"


#define SPOT_LIGHT 0
#define POINT_LIGHT 1
#define DIRECTIONAL_LIGHT 2

class GPUFBO;

struct Light
{
	glm::vec4 position;
	glm::vec4 color;
	glm::vec4 direction;
	glm::vec4 info;//.x = type lumiere
	// Add matrices here
	//glm::mat4 worldToLight;
	//glm::mat4 worldToLight2;
};


void printLight(Light* L);


class LightNode : public Node
{
public:
	

	LightNode(std::string name,const glm::vec4 & color = glm::vec4(1.0f));

	Light getParams();
	void setParams(const Light & p);
	
	glm::vec3 getPosition();
	void setPosition(const glm::vec4 & p);
	
	glm::vec4 getColor();
	void setColor(const glm::vec4 & c);

	float getType();
	void setType(unsigned int kind);

	float getAngle();
	void setAngle(float a);

	glm::vec4 getDirection();
	void setDirection(const glm::vec4 & dir); 

//	void initShadowMap(unsigned int size);// la fonction qui génére les cameras et les FBO aux besoin
	
//	unsigned int getNbrShadowmap();
//	void enableFBO(unsigned int i);
//	void disableFBO(unsigned int i);

//	Camera * getCam(unsigned int i);

private:
	Light params;
	Camera	* cam,*cam2;
	GPUFBO *mFBO,*mFBO2;
	int shadowMapSize;
	bool needUpdate;
};
	

#endif
