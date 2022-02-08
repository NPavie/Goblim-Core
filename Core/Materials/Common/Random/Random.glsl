

/*		-- headers Random --
	void initRandom(tf seed)
	float random();
	vec2 randomVec2();
	vec3 randomVec3();
	float randomIn(float min, float max);
	vec2 randomIn(vec2 min, vec2 max);
	vec3 randomIn(vec3 min, vec3 max);
*/

// variables globales
uint seed;
// Définition de la graine de départ
void seeding(uint s)
{
  seed = s + 1200;
}

uint next()
{
	seed *= 3039177861u;
	return seed;
}

uint getState()
{
	return seed;
}

void setState(uint state)
{
	seed = state;
}


/*
*		@brief Générateurs de nombres aléatoires 
*/

float random()
{
	float f = float(next())/4294967295u;
	return(f);
	// 1 seed shift
}

vec2 randomVec2()
{
	float f1 = float(next())/4294967295u;
	float f2 = float(next())/4294967295u;
	return(vec2(f1,f2));
	// 2 seed shift
}

vec3 randomVec3()
{
	float f1 = float(next())/4294967295u;
	float f2 = float(next())/4294967295u;
	float f3 = float(next())/4294967295u;
	return(vec3(f1,f2,f3));
	// 3 seed shift
}


float randomIn(float min, float max)
{
	return min + (random() * (max - min));	
	// 1 seed shift
}

vec2 randomIn(vec2 min, vec2 max)
{
	return min + (randomVec2() * (max - min));
	// 2 seed shift	
}

vec3 randomIn(vec3 min, vec3 max)
{
	return min + (randomVec3() * (max - min));
	// 3 seed shift	
}



// ---- Création de la graine grace à un nombre de morton
// Morton code 2D
uint morton(uint x, uint y)
{
	uint z = 0;
	uint one = 1;
	for (uint i = 0; i < (1 * 8); ++i) {
		z = z | ((x & (one << i)) << i) | ((y & (one << i)) << (i + one));
	}
	return z;
}

uint morton(uint x, uint y,uint z)
{
	uint w = 0;
	uint one = 1;
	uint two = 2;
	for (uint i = 0; i < (1 * 8); ++i) {
		w = w | ((x & (one << i)) << i) | ((y & (one << i)) << (i + one)) | ((z & (one << i)) << (i + two));
	}
	return w;
}
//inline uint64_t mortonEncode_for(unsigned int x, unsigned int y, unsigned int z){
//    uint64_t answer = 0;
//    for (uint64_t i = 0; i < (sizeof(uint64_t)* CHAR_BIT)/3; ++i) {
//        answer |= ((x & ((uint64_t)1 << i)) << 2*i) | ((y & ((uint64_t)1 << i)) << (2*i + 1)) | ((z & ((uint64_t)1 << i)) << (2*i + 2));
//    }
//    return answer;
//}

// Distribution de poisson
uint poisson(float mean)
{
	float g = exp(-mean);
	uint em = 0;
	double t = random();
	while (t > g) {
		em = em+1;
		t *= random();
	}
	return em;
}


/* Classic morton code */
// "Insert" a 0 bit after each of the 16 low bits of x
uint Part1By1(uint x)
{
  uint xM = uint(x);
  uint huit = 8;
  uint quatre = 4;
  uint deux = 2;
  uint un = 1;

  xM &= 0x0000ffff;                  // x = ---- ---- ---- ---- fedc ba98 7654 3210
  xM = (xM ^ (xM <<  huit)) & 0x00ff00ff; // x = ---- ---- fedc ba98 ---- ---- 7654 3210
  xM = (xM ^ (xM <<  quatre)) & 0x0f0f0f0f; // x = ---- fedc ---- ba98 ---- 7654 ---- 3210
  xM = (xM ^ (xM <<  deux)) & 0x33333333; // x = --fe --dc --ba --98 --76 --54 --32 --10
  xM = (xM ^ (xM <<  un)) & 0x55555555; // x = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
  return xM;
}

// "Insert" two 0 bits after each of the 10 low bits of x
uint Part1By2(uint x)
{
  uint xM = uint(x);
  uint seize = 16;
  uint huit = 8;
  uint quatre = 4;
  uint deux = 2;
  xM &= 0x000003ff;                  // x = ---- ---- ---- ---- ---- --98 7654 3210
  xM = (xM ^ (xM << seize)) & 0xff0000ff; // x = ---- --98 ---- ---- ---- ---- 7654 3210
  xM = (xM ^ (xM <<  huit)) & 0x0300f00f; // x = ---- --98 ---- ---- 7654 ---- ---- 3210
  xM = (xM ^ (xM <<  quatre)) & 0x030c30c3; // x = ---- --98 ---- 76-- --54 ---- 32-- --10
  xM = (xM ^ (xM <<  deux)) & 0x09249249; // x = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
  return x;
}


uint morton2D(uint x, uint y)
{
	uint un = 1;
  return (Part1By1(y) << un) + Part1By1(x);
}

uint morton3D(uint x, uint y, uint z)
{
	 uint deux = 2;
	uint un = 1;
  return (Part1By2(z) << 2) + (Part1By2(y) << 1) + Part1By2(x);
}


/*
*		@brief Initialisation du Générateur
*/

void initRandom(float seed)
{
	uint s = uint(seed);
	if (s == uint(0)) s = uint(1);
	seeding(s);
}

void initRandom(vec2 seedCell)
{
	
	// identifiants de la cellule	
	uint cellX = uint(seedCell.x);
	uint cellY = uint(seedCell.y);
	uint s = morton(cellX, cellY) ; // nonperiodic noise


	if (s == uint(0)) s = uint(1);
	seeding(s);
}


void initRandom(vec3 seedCell)
{
	// identifiants de la cellule	
	//uint cellX = uint(seedCell.x);
	//uint cellY = uint(seedCell.y);
	//uint cellZ = uint(seedCell.z);
	//uint s = morton3D(cellX, cellY, cellZ) ; // nonperiodic noise

	uvec3 cell = uvec3(seedCell);
	uint s = morton3D(cell) ;

	if (s == uint(0)) s = uint(1);
	seeding(s);
}

vec3 randomSignVec3()
{
	vec3 randomSigne = randomIn(vec3(-1),vec3(1));
	randomSigne.x = randomSigne.x > 0 ? 1.0 : -1.0; 
	randomSigne.y = randomSigne.y > 0 ? 1.0 : -1.0; 
	randomSigne.z = randomSigne.z > 0 ? 1.0 : -1.0; 

	return randomSigne;
}

vec2 randomSignVec2()
{
	vec2 randomSigne = randomIn(vec2(-1),vec2(1));
	randomSigne.x = randomSigne.x > 0 ? 1.0 : -1.0; 
	randomSigne.y = randomSigne.y > 0 ? 1.0 : -1.0; 
	
	return randomSigne;
}



vec3 randomInFrame(vec3 center, vec3 minHalfVector, vec3 maxHalfVector)
{
	return center + randomSignVec3() * randomIn(minHalfVector,maxHalfVector);
}

vec2 randomInFrame(vec2 center, vec2 minHalfVector, vec2 maxHalfVector)
{
	return center + randomSignVec2() * randomIn(minHalfVector,maxHalfVector);
}

vec3 randomInPartialSphere(vec3 center, float radiusMin, float radiusMax)
{
	return center + normalize(randomSignVec3()) * randomIn(radiusMin, radiusMax);
}

vec3 randomInPartialCylinder(vec3 center, float radiusMin, float radiusMax, float heightMin, float heightMax)
{
	return center + normalize(randomSignVec3()) * vec3( vec2( randomIn(radiusMin, radiusMax) ), randomIn(heightMin,heightMax));
}

