
#include "RandomGenerator.h"

using namespace glm;


RandomGenerator::~RandomGenerator()
{


}


/*
*	uninitialized Random Generator 
*/
RandomGenerator::RandomGenerator()
{

}
	


/*
*	Random Generator initialized with a integer identifier
*/
RandomGenerator::RandomGenerator(float seed)
{
	this->initRandom(seed);
}

	/*
	*	Random Generator initialized with a 2D cell identifier
	*/
RandomGenerator::RandomGenerator(glm::vec2 seedCell)
{
	this->initRandom(seedCell);	
}

/*
*	Random Generator initialized with a 3D cell identifier
*/
RandomGenerator::RandomGenerator(glm::vec3 seedCell)
{
	this->initRandom(seedCell);	
}



// Définition de la graine de départ
void RandomGenerator::seeding(unsigned int s)
{
  seed = s;
}

unsigned int RandomGenerator::next()
{
	seed *= 3039177861u;
	return seed;
}


/*
*		@brief Générateurs de nombres aléatoires 
*/

float RandomGenerator::random()
{
	float f = float(next())/4294967295u;
  
	return(f);
	// 1 seed shift
}

vec2 RandomGenerator::randomVec2()
{
	float f1 = float(next())/4294967295u;
	float f2 = float(next())/4294967295u;
	return(vec2(f1,f2));
	// 2 seed shift
}

vec3 RandomGenerator::randomVec3()
{
	float f1 = float(next())/4294967295u;
	float f2 = float(next())/4294967295u;
	float f3 = float(next())/4294967295u;
	return(vec3(f1,f2,f3));
	// 3 seed shift
}


float RandomGenerator::randomIn(float min, float max)
{
	return min + (random() * (max - min));	
	// 1 seed shift
}

vec2 RandomGenerator::randomIn(vec2 min, vec2 max)
{
	return min + (randomVec2() * (max - min));
	// 2 seed shift	
}

vec3 RandomGenerator::randomIn(vec3 min, vec3 max)
{
	return min + (randomVec3() * (max - min));
	// 3 seed shift	
}



/* Classic morton code */
// "Insert" a 0 bit after each of the 16 low bits of x
unsigned int RandomGenerator::Part1By1(unsigned int x)
{
  unsigned int xM = (unsigned int)(x);
  unsigned int huit = 8;
  unsigned int quatre = 4;
  unsigned int deux = 2;
  unsigned int un = 1;

  xM &= 0x0000ffff;                  // x = ---- ---- ---- ---- fedc ba98 7654 3210
  xM = (xM ^ (xM <<  huit)) & 0x00ff00ff; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
  xM = (xM ^ (xM <<  quatre)) & 0x0f0f0f0f; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
  xM = (xM ^ (xM <<  deux)) & 0x33333333; // x = --fe --dc --ba --98 --76 --54 --32 --10
  xM = (xM ^ (xM <<  un)) & 0x55555555; // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
  return xM;
}

// "Insert" two 0 bits after each of the 10 low bits of x
unsigned int RandomGenerator::Part1By2(unsigned int x)
{
  unsigned int xM = (unsigned int)(x);
  unsigned int seize = 16;
  unsigned int huit = 8;
  unsigned int quatre = 4;
  unsigned int deux = 2;
  xM &= 0x000003ff;                  // x = ---- ---- ---- ---- ---- --98 7654 3210
  xM = (xM ^ (xM << seize)) & 0xff0000ff; // x = ---- --98 ---- ---- ---- ---- 7654 3210
  xM = (xM ^ (xM <<  huit)) & 0x0300f00f; // x = ---- --98 ---- ---- 7654 ---- ---- 3210
  xM = (xM ^ (xM <<  quatre)) & 0x030c30c3; // x = ---- --98 ---- 76-- --54 ---- 32-- --10
  xM = (xM ^ (xM <<  deux)) & 0x09249249; // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
  return x;
}


unsigned int RandomGenerator::morton2D(unsigned int x, unsigned int y)
{
	unsigned int un = 1;
  return (Part1By1(y) << un) + Part1By1(x);
}



// ---- Création de la graine grace à un nombre de morton
// Morton code 2D
unsigned int RandomGenerator::morton(unsigned int x, unsigned int y)
{
	unsigned int z = 0;
	unsigned int one = 1;
	unsigned int test = 1*8;
	for (unsigned int i = 0; i < (1 * 8); ++i) {
		z = z | ((x & (one << i)) << i) | ((y & (one << i)) << (i + one));
	}
	return z;
}


// Distribution de poisson
unsigned int RandomGenerator::poisson(float mean)
{
	float g = exp(-mean);
	unsigned int em = 0;
	double t = random();
	while (t > g) {
		em = em+1;
		t *= random();
	}
	return em;
}


unsigned int RandomGenerator::morton3D(unsigned int x, unsigned int y, unsigned int z)
{
	  unsigned int deux = 2;
	unsigned int un = 1;
  return (Part1By2(z) << 2) + (Part1By2(y) << 1) + Part1By2(x);
}

// Morton code 3D
unsigned int RandomGenerator::morton3D(glm::vec3 v)
{
	unsigned int nbM = 0;
	unsigned int zero = 0;
	unsigned int one = 1;
	unsigned int two = 2;
	unsigned int x = (unsigned int)(v.x);
	unsigned int y = (unsigned int)(v.y);
	unsigned int z = (unsigned int)(v.z);
	for (unsigned int i = 0; i < (1 * 16); ++i) {
		nbM = zero | ((x & (one << i)) << i) | ((y & (one << i)) << (i + one)) | ((y & (one << i)) << (i + two)) ;
	}
	return nbM;
}
/*
*		@brief Initialisation du Générateur
*/

void RandomGenerator::initRandom(float seed)
{
	unsigned int s = (unsigned int)(seed);
	if (s == (unsigned int)(0)) s = (unsigned int)(1);
	seeding(s);
}

void RandomGenerator::initRandom(vec2 seedCell)
{
	
	// identifiants de la cellule	
	unsigned int cellX = (unsigned int)(seedCell.x);
	unsigned int cellY = (unsigned int)(seedCell.y);
	unsigned int s = morton2D(cellX, cellY) ; // nonperiodic noise
	
	if (s == (unsigned int)(0)) s = (unsigned int)(1);
	seeding(s);
}


void RandomGenerator::initRandom(vec3 seedCell)
{
	
	// identifiants de la cellule	
	unsigned int cellX = (unsigned int)(seedCell.x);
	unsigned int cellY = (unsigned int)(seedCell.y);
	unsigned int cellZ = (unsigned int)(seedCell.z);
	unsigned int s = morton3D(cellX, cellY, cellZ) ; // nonperiodic noise
	
	if (s == (unsigned int)(0)) s = (unsigned int)(1);
	seeding(s);
}
