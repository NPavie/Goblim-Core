#ifndef PARTICLE_MANAGER_HPP_
#define PARTICLE_MANAGER_HPP_

#include <glm/glm.hpp>

struct Particle {
	glm::vec3 pos, vel, acc;
	unsigned char r, g, b, a;
	float size, angle, weight;
	int life;
};


class ParticleManager {
public:
	ParticleManager ();

private:
	static const int maxParticles = 12000;
	
	Particle particles[maxParticles];
	
	int indexLastParticle = 0;


};

#endif