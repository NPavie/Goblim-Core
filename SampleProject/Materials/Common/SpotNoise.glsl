
#ifndef PI
	#define PIx2e2 39.4784176044
	#define PIx2e3_2 15.7496099457
	#define PIx2 6.28318530718
	#define PI 3.14159265359
	#define PI_2 1.57079632679
	#define PI_3 1.0471975512
	#define PI_4 0.78539816339
	// TODO préacalculer sqrt(2PI), 2PI et (2PI)^(3/2)

#endif

#ifndef _SPOTNOISE_
#define _SPOTNOISE_


#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Random/Random"
#line 20



struct Gaussian
{
	mat4 V;
	vec4 color;
};

/** @brief Gaussienne simple : G(X) = K * e^(-0.5 * X^T*V*X )
	@param evaluatedPosition 	Vecteur X (Position évaluée dans l'espace du noyau)
	@param param				Paramètres de la gaussienne (color et matrice V représentant une isosurface quadratic)
*/
float gaussianFunc(in vec3 evaluatedPosition, in Gaussian param)
{
	
	float d = dot(
		vec4(evaluatedPosition, 1.0), 
		param.V * vec4(evaluatedPosition, 1.0));

	float fallOffLimit = 4.27; // Full width at Tenth of magnitude
	float fallOfParam = max(0.01,param.color.a);
	float fallOffSize = 1.0f /  max(0.01,fallOfParam);
	float d_prime = max(0.0f, 
		- (1.0 - fallOfParam) * sqrt(fallOffLimit) * fallOffSize 
		+ d * fallOffSize);
	
	return clamp(
		1.0f * exp(-0.5 * d_prime) ,
		0.0f,1.0f)	;
}

float gaussianFuncNormal(in vec3 evaluatedPosition, in Gaussian param)
{

	float d = dot(
		vec4(evaluatedPosition, 1.0), 
		param.V * vec4(evaluatedPosition, 1.0));
	//float fallOffLimit = dot(vec4(1.0), param.V * vec4(1.0));
	float fallOffLimit = 4.27; // Full width at Tenth of magnitude
	float fallOfParam = max(0.01,param.color.a);
	float fallOffSize = 1.0f /  max(0.01,fallOfParam);
	float d_prime = max(0.0f, 
		- (1.0 - fallOfParam) * sqrt(fallOffLimit) * fallOffSize 
		+ d * fallOffSize);

	return clamp(
		1.0f * exp(-0.5 * d_prime) ,
		0.0f,1.0f)	;
}

/** @brief Construction d'une matrice représentant l'iso surface d'une ellipse
	@param shift	Translation de l'ellipse
	@param scale	Vecteur d'échelle des axes de l'ellipse
	@param rotation	angles de rotations de l'ellipse autour des axes X,Y et Z de l'espace d'évaluation (R = Rz * Ry * Rx)
	@return la matrice (M*R*S)^T * (M*R*S) avec M la matrice de translation, R la matrice de rotation et S la matrice d'échelle
*/
mat4 ellipseIsoSurfaceMatrix(in vec3 shift, in vec3 scale, in vec3 rotation)
{
	// Ellipse = specific quadratic iso surface, scaled along its axis
	// -> Q = (ShiftMat * RotMat * ScaleMat)^-T * (ShiftMat * RotMat * ScaleMat)^-1
	// cosines
	vec3 c = cos(rotation);
	// sines
	vec3 s = sin(rotation);
	// REMINDER : matrices are column order in glsl (a vec4 is a column)
	// This a simple predevelopped form of the ShiftMat * RotMat * ScaleMat operation (with RotMat = RotAroundZ * RotAroundY * RotAroundX)
	// FLAGUNSURE
	mat4 MRS = mat4(
		scale.x * vec4(
			(c.z * c.y), 
			(s.z * c.y), 
			-s.y, 
			0.0 ),
		scale.y * vec4(
			(c.z * s.y * s.x - s.z * c.x) , 
			(s.z * s.y * s.x + c.z * c.x ), 
			(c.y * s.x), 
			0.0 ),
		scale.z * vec4(
			(c.z * s.y * c.x + s.z * s.x) , 
			(s.z * s.y * c.x - c.z * s.x ), 
			(c.y * c.x), 
			0.0),
		vec4(shift,1.0));
	MRS = inverse(MRS);
	return transpose(MRS) * MRS;
	// distance to the surface ellipse surface is given by X^t * Q * X which is equivalent to dot(X, Q*X)
}

struct Harmonic
{
	vec4 thetaPhiFrequency;
};

// FLAGUNSURE : pas la bonne fonction
float harmonicFunc(in vec3 evaluatedPosition, in Harmonic param)
{
	return clamp(
		cos(
			PIx2 * param.thetaPhiFrequency.z 
			* ( evaluatedPosition.x * cos(param.thetaPhiFrequency.x) 
			//* sin(param.thetaPhiFrequency.y) 
			+ evaluatedPosition.y * sin(param.thetaPhiFrequency.x)
			// * sin(param.thetaPhiFrequency.y) 
			//+ evaluatedPosition.z * cos(param.thetaPhiFrequency.y) 
			)),
		-1.0f, 1.0f);
}

struct Constant
{
	float constantValue;
};

// Inline if possible
float constantFunc(in vec3 evaluatedPosition, in Constant param)
{
	return param.constantValue;
}


///////// BUFFERS 
// buffer of Gaussian function parameters
layout (std430,binding=3) buffer gaussianDataBuffer
{
	Gaussian gaussian[];
};

// For later usage and gabor kernels creations : buffer of harmonic functions
layout (std430,binding=4) buffer harmonicDataBuffer
{
	Harmonic harmonic[];
};
// Buffer of constant values for more complex spot creations
layout (std430,binding=5) buffer constantDataBuffer
{
	Constant constant[];
};

// Tableau des paramètres d'une distribution (float pour alignement)
struct Distribution
{
	float param[24];
};

// SSBO des distribution
layout (std430,binding=6) buffer distribDataBuffer
{
	// distrib : 
	// response.xyz,
	// weigtRange.xy, 
	// posMinCenterMax[3][3],
	// rotationsMinMax.xyzw,
	// scaleMin.xyz,
	// scaleMax.xyz
	Distribution distrib[];
};

// Caching the distribution used for easier usage
struct CachedDistribution
{
	vec3 responseSize; // 0 1 2

	// random weight range
	float w_min; // 3
	float w_max; // 4

	// random position range
	vec3 p_min; // 5 6 7
	vec3 p_center; // 8 9 10
	vec3 p_max; // 11 12 13

	// random rotation range
	vec4 rotate_min_max; // 14 15 // 16 17

	// random scale range
	vec3 scale_min; // 18 19 20
	vec3 scale_max; // 21 22 23
};

CachedDistribution fromDistributionBuffer(Distribution dist)
{
	CachedDistribution temp;
	temp.responseSize = vec3(dist.param[0],dist.param[1],dist.param[2]);
	temp.w_min = dist.param[3];
	temp.w_max = dist.param[4];
	temp.p_min = vec3(dist.param[5],dist.param[6],dist.param[7]);
	temp.p_center = vec3(dist.param[8],dist.param[9],dist.param[10]);
	temp.p_max = vec3(dist.param[11],dist.param[12],dist.param[13]);
	temp.rotate_min_max = vec4(
		dist.param[14],
		dist.param[15],
		dist.param[16],
		dist.param[17]);
	temp.scale_min = vec3(dist.param[18],dist.param[19],dist.param[20]);
	temp.scale_max = vec3(dist.param[21],dist.param[22],dist.param[23]);
	return temp;
}
//////////////

// Data buffer
layout (std430,binding=7) buffer spotDataBuffer
{
	int spotData[];
	// spotData : nbFunction_S0, F_1[Operand_1,typeFunction_1,paramId_1], F_2[], nbFunction_S1... 	
	
};

// Index buffers of spots
layout (std430,binding=8) buffer spotIndexBuffer
{
	int spotIndex[];
	// Index of the first spotData element for each spot
};

layout (std430,binding=9) buffer sizeOfArraysBuffer
{
	int gaussianArraySize;
	int harmonicArraySize;
	int constantArraySize;
	int distribArraySize;
	int spotDataArraySize;
	int spotIndexArraySize;
	int spotsTreeNodeDataArraySize;
	int spotsTreeNodeIndexArraySize;
};

// Solution pour la configuration de la puissance du bruit : 

layout (std430,binding=10) buffer weightsDataBuffer
{
	float spotWeight[];				// w_i associé au bruit n_i généré par un spot i
};



// Evaluate a node response from its ID in the spot index
// pas la version la plus opti qui soit
float spotResponse(vec3 p,int spotID)
{
	float res = 0.0f;
	int spotFirstDataIndex = spotIndex[spotID];
	int spot_nbFunction = spotData[spotFirstDataIndex];
	int currentIndex = spotFirstDataIndex+1;
	for(int f = 0; f < spot_nbFunction;++f)
	{

		int operand = spotData[currentIndex + (3*f)];
		int function_type = spotData[currentIndex + (3*f) + 1];
		int param = spotData[currentIndex + (3*f) + 2];
		float func_eval = 0.0f;
		switch(function_type)
		{
			case 0: // gaussian
				func_eval = gaussianFunc(p,gaussian[param]);
				break;
			case 1: //harmonic
				func_eval = harmonicFunc(p,harmonic[param]);
				break;
			case 2: //constant
				func_eval = constantFunc(p,constant[param]);
				break;
			default:
				func_eval = 0;
				break;
		}
		switch(operand)
		{
			case 0:
			default:
				res += func_eval;
				break;
			case 1:
				res -= func_eval;
				break;
			case 2:
				res *= func_eval;
				break;
		}

	}
	return clamp(res,-1.0,1.0);
}

vec4 gaussiansSpot(vec3 p,uint spotID)
{
	vec4 res = vec4(0.0f);
	int spotFirstDataIndex = spotIndex[spotID];
	int spot_nbFunction = spotData[spotFirstDataIndex];
	int currentIndex = spotFirstDataIndex+1;
	// Opti possible : mettre en cache les parametres des gaussiennes
	 
	for(int f = 0; f < spot_nbFunction;++f)
	{
		int param = spotData[currentIndex + (3*f) + 2];
		Gaussian G = gaussian[param];
		res += vec4(G.color.rgb,1.0f) * gaussianFunc(p,G);
	}
	res = res.a > 1.0 ? res / res.a : res;
	res.xyz = res.a > 0.0 ? res.xyz / res.a : res.xyz;
	return res; // normalized color of the resulting spot with res.a = alpha
}

vec4 gaussiansNormal(vec3 p,uint spotID)
{
	vec4 res = vec4(0.0f);
	int spotFirstDataIndex = spotIndex[spotID];
	int spot_nbFunction = spotData[spotFirstDataIndex];
	int currentIndex = spotFirstDataIndex+1;
	// Opti possible : mettre en cache les parametres des gaussiennes
	 
	for(int f = 0; f < spot_nbFunction;++f)
	{
		int param = spotData[currentIndex + (3*f) + 2];
		Gaussian G = gaussian[param];
		res += vec4(G.color.rgb,1.0f) * gaussianFunc(p,G);
	}
	res = res.a > 1.0 ? res / res.a : res;
	res.xyz = res.a > 0.0 ? res.xyz / res.a : res.xyz;
	return res; // normalized color of the resulting spot with res.a = alpha
}

// For anisotropic noise (randomly rotated kernel)
mat4 R_j(vec4 rotationMinMax)
{

	vec2 randomRotate = rotationMinMax.xy 
		+ (rotationMinMax.zw - rotationMinMax.xy) * random();
	vec3 rotation = vec3(randomRotate.x,0 , randomRotate.y);
	// cosines
	vec3 c = cos(rotation);
	// sines
	vec3 s = sin(rotation);
	// REMINDER : matrices are column order in glsl (a vec4 is a column)
	// FLAGUNSURE
	return mat4(
		vec4((c.z * c.y), (s.z * c.y), -s.y, 0.0 ),
		vec4((c.z * s.y * s.x - s.z * c.x) , (s.z * s.y * s.x + c.z * c.x ), (c.y * s.x), 0.0 ),
		vec4((c.z * s.y * c.x + s.z * s.x) , (s.z * s.y * c.x - c.z * s.x ), (c.y * c.x), 0.0),
		vec4(0.0,0.0,0.0,1.0));
}

mat4 S_j(vec3 scaleMin, vec3 scaleMax)
{
	vec3 randomScale = randomIn(scaleMin,scaleMax);
	mat4 S = mat4(1.0);
	S[0].x = randomScale.x;
	S[1].y = randomScale.y;
	S[2].z = randomScale.z;
	return S;
}



float poissonProcessProfile(vec2 p, int impulsePerCellAxis){
	float res = 0.0f;
	
	// Current evaluation cell
	ivec2 cell = ivec2(floor(p));
	
	ivec2 currentCell;
	for(currentCell.x = cell.x - 1; currentCell.x <= cell.x +1; ++currentCell.x)
	for(currentCell.y = cell.y - 1; currentCell.y <= cell.y +1; ++currentCell.y)
	{
		// Init PRNG
		uint cx = (currentCell.x) < 0 ? 4294967295u - abs(currentCell.x) : uint(currentCell.x);
		uint cy = currentCell.y < 0 ? 4294967295u - abs(currentCell.y) : uint(currentCell.y);
		seeding(morton(cx,cy));


		ivec2 imp_cell = ivec2(0);
		float subCellSize = 1.0 / impulsePerCellAxis;
		for(imp_cell.x = 0; imp_cell.x < impulsePerCellAxis; ++imp_cell.x)
		for(imp_cell.y = 0; imp_cell.y < impulsePerCellAxis; ++imp_cell.y)
		{
			vec2 subCell = currentCell + (vec2(imp_cell) / impulsePerCellAxis); 
			// Poisson process : each impulse is randomly distributed in a sub-cell of size (1.0 / impulsePerCellAxis) of the current cell
			vec2 impulsePos = randomIn(subCell, subCell + vec2(subCellSize * 0.5f) );

			mat4 rotate = mat4(1.0);
			rotate = R_j(vec4(0,0,0,PIx2));

			float d = length(p - impulsePos) < (0.25/impulsePerCellAxis) ? 1.0 : 0.0;
			res += randomIn(-1.0,1.0) * d;
		}

	}

	return 0.5f + 0.5f * clamp(res,-1.0f,1.0f);
}

float spotNoise2D(vec2 p, uint spotID, int impulsePerCellAxis){

	float res = 0.0f;
	
	// Current evaluation cell
	ivec2 cell = ivec2(floor(p));
	
	ivec2 currentCell;
	for(currentCell.x = cell.x - 1; currentCell.x <= cell.x +1; ++currentCell.x)
	for(currentCell.y = cell.y - 1; currentCell.y <= cell.y +1; ++currentCell.y)
	{
		// Init PRNG
		uint cx = currentCell.x < 0 ? 4294967295u + currentCell.x : uint(currentCell.x);
		uint cy = currentCell.y < 0 ? 4294967295u + currentCell.y : uint(currentCell.y);
		seeding(morton(cx,cy));

		float subCellSize = 1.0 / impulsePerCellAxis;
		ivec2 imp_cell;
		for(imp_cell.x = 0; imp_cell.x < impulsePerCellAxis; ++imp_cell.x)
		for(imp_cell.y = 0; imp_cell.y < impulsePerCellAxis; ++imp_cell.y)
		{
			vec2 subCell = currentCell + (vec2(imp_cell) / impulsePerCellAxis); 
			// Poisson process : each impulse is randomly distributed in a sub-cell of size (1.0 / impulsePerCellAxis) of the current cell
			vec2 impulsePos = randomIn(subCell, subCell + vec2(subCellSize * 0.5f) );

			// Check

			mat4 rotate = mat4(1.0);
			rotate = R_j(vec4(0,0,0,0));

			res += randomIn(-1.0,1.0) * gaussiansSpot((vec4(p - impulsePos,0,1) * rotate).xyz , spotID);

		}

	}
	// Normalement, il faudrait minimiser l'énergie du noyau pour miex controler le résultat, mais pas trop le temps
	return 0.5f + 0.5f * clamp(res * spotWeight[spotID]  * 0.33f,-1.0f,1.0f);

}

float NonUniformDistributionProfileWithKroenecker(vec2 p, uint distribProfileID, int impulsePerCellAxis) {

	float res = 0.0f;
	
	// Current evaluation cell
	ivec2 cell = ivec2(floor(p));
	
	ivec2 currentCell;
	for(currentCell.x = cell.x - 1; currentCell.x <= cell.x +1; ++currentCell.x)
	for(currentCell.y = cell.y - 1; currentCell.y <= cell.y +1; ++currentCell.y)
	{
		// Init PRNG
		uint cx = (currentCell.x) < 0 ? 4294967295u - abs(currentCell.x) : uint(currentCell.x);
		uint cy = currentCell.y < 0 ? 4294967295u - abs(currentCell.y) : uint(currentCell.y);
		seeding(morton(cx,cy));

		int missedImpulses = 0;
		for(int imp = 0; imp < impulsePerCellAxis * impulsePerCellAxis; ++imp)
		{
			vec2 impulsePos = currentCell + randomVec2();
			float impulseDensityTest = random();

			// density profile in 0-1 from a spot function
			float densityProfile = gaussiansSpot((vec4(fract(impulsePos) - 0.5,0,1)).xyz , distribProfileID).a;

			float d = length(p - impulsePos) < (0.25/impulsePerCellAxis) ? 1.0 : 0.0;
			res += (impulseDensityTest < densityProfile ? 1.0f : 0.0f) * (randomIn(0.0,1.0)) * d;
			missedImpulses += (impulseDensityTest < densityProfile ? 0 : 1);

		}
	}
	return clamp(res,-1.0f,1.0f);

}

// Version avec kroenecker delta
float NonUniformSpotNoiseWithKroenecker(vec2 p, uint spotID, uint distribProfileID, int impulsePerCellAxis) {

	float res = 0.0f;
	
	// Current evaluation cell
	ivec2 cell = ivec2(floor(p));
	
	ivec2 currentCell;
	for(currentCell.x = cell.x - 1; currentCell.x <= cell.x +1; ++currentCell.x)
	for(currentCell.y = cell.y - 1; currentCell.y <= cell.y +1; ++currentCell.y)
	{
		// Init PRNG
		uint cx = (currentCell.x) < 0 ? 4294967295u - abs(currentCell.x) : uint(currentCell.x);
		uint cy = currentCell.y < 0 ? 4294967295u - abs(currentCell.y) : uint(currentCell.y);
		seeding(morton(cx,cy));

		int missedImpulses = 0;
		for(int imp = 0; imp < impulsePerCellAxis * impulsePerCellAxis; ++imp)
		{
			vec2 impulsePos = currentCell + randomVec2();
			float impulseDensityTest = random();

			// dentiy profile
			float densityProfile = gaussiansSpot((vec4(fract(impulsePos) - 0.5,0,1)).xyz , distribProfileID).a;
			res += (impulseDensityTest < densityProfile ? 1.0f : 0.0f) * (randomIn(0.0,1.0))  * gaussiansSpot((vec4(p - impulsePos,0,1) ).xyz , spotID).a;
			
			missedImpulses += (impulseDensityTest < densityProfile ? 0 : 1);

		}
	}
	// 
	//return 0.5f + 0.5f * clamp(res * spotWeight[spotID] * 0.33f,-1.0f,1.0f);
	return clamp(res * spotWeight[spotID],-1.0f,1.0f);

}

// Version periodic controlé
float controlledDistributionProfile(vec2 p, CachedDistribution dist)
{
	return all(greaterThanEqual(p,dist.p_center.xy + dist.p_min.xy)) 
			&& all(lessThanEqual(p,dist.p_center.xy + dist.p_max.xy)) ? 
			1.0 : 0.0;
}

float NonUniformControledDistributionProfile(vec2 p, int distribID) {

	CachedDistribution dist = fromDistributionBuffer(distrib[distribID]);
	float res = 0.0f;
	vec2 point = p * dist.responseSize.xy;
	// Current evaluation cell

	ivec2 cell = ivec2(floor(point));

	ivec2 currentCell;
	for(currentCell.x = cell.x - 1; currentCell.x <= cell.x +1; ++currentCell.x)
	for(currentCell.y = cell.y - 1; currentCell.y <= cell.y +1; ++currentCell.y)
	{
		// Init PRNG
		uint cx = (currentCell.x) < 0 ? 4294967295u - abs(currentCell.x) : uint(currentCell.x);
		uint cy = currentCell.y < 0 ? 4294967295u - abs(currentCell.y) : uint(currentCell.y);
		seeding(morton(cx,cy));

		for(int imp = 0; imp < dist.responseSize.z; ++imp)
		{
			vec2 impulsePos = currentCell + dist.p_center.xy + randomIn(dist.p_min.xy,dist.p_max.xy);
			float densityTest = randomIn(0.1f,0.9f);
			int impulseSpotID = int(randomIn(0,1));

			mat4 Rj = R_j(vec4(0,0,0,0));
			mat4 Sj = S_j(vec3(1.0), vec3(1.0));

			float d = length(point - impulsePos) < 0.1 ? 1.0 : 0.0;
			res += randomIn(dist.w_min,dist.w_max) * d;
			
		}
	}
	return 0.5f + 0.5f * clamp(res,-1.0f,1.0f);

}




vec4 NonUniformControlledOrderedSpotNoise(vec2 p, int firstSpotID, int nbSpotPerAxis, int distribID) {

	CachedDistribution dist = fromDistributionBuffer(distrib[distribID]);
	
	vec4 res = vec4(0.0f);
	vec2 point = p * dist.responseSize.xy;
	// Current evaluation cell
	ivec2 cell = ivec2(floor(point));
	

	ivec2 currentCell;
	for(currentCell.x = cell.x - 1; currentCell.x <= cell.x +1; ++currentCell.x)
	for(currentCell.y = cell.y - 1; currentCell.y <= cell.y +1; ++currentCell.y)
	{
		// Init PRNG
		uint cx = currentCell.x < 0 ? 4294967295u - abs(currentCell.x) : uint(currentCell.x);
		uint cy = currentCell.y < 0 ? 4294967295u - abs(currentCell.y) : uint(currentCell.y);
		seeding(morton(cx,cy));

		uint impulseSpotID = (cx % nbSpotPerAxis) + nbSpotPerAxis * (cy % nbSpotPerAxis);
		//changing the distribution process : distrib in 0-1 and retransforn in 2d
		for(int imp = 0; imp < dist.responseSize.z; ++imp)
		{
			mat4 Rj = R_j(vec4(0,0,0,PIx2));
			mat4 Sj = S_j(vec3(1.0), vec3(1.0));
			vec2 impulsePos = currentCell + dist.p_center.xy + randomIn(dist.p_min.xy,dist.p_max.xy);
			vec4 spotColor = gaussiansSpot((vec4(point - impulsePos,0,1) ).xyz , impulseSpotID);
			res += randomIn(dist.w_min,dist.w_max) * vec4(spotColor.rgb * spotColor.a, spotColor.a) ;
		}
	}
	res = res.a > 1.0 ? res / res.a : res; // Renormalize if >1
	//res.rgb = res.a > 0.0 ? res.rgb / res.a : res.rgb; // Recompute true color if < 1
	//res.rgb *= res.a;
	//res = res * spotWeight[spotID];
	//res.xyz *= (res.a * spotWeight[spotID]);
	//res.a *= spotWeight[spotID];
	
	return res;

}



vec4 NonUniformControlledRandomizedSpotNoise(vec2 p, int firstSpotID, int nbSpot, int distribID ) {

	CachedDistribution dist = fromDistributionBuffer(distrib[distribID]);
	
	vec4 res = vec4(0.0f);
	vec2 point = p * dist.responseSize.xy;
	// Current evaluation cell
	ivec2 cell = ivec2(floor(point));
	
	vec2 realPoint  = p / 4.0f;

	float localVariation = 0.0;

	ivec2 currentCell;
	for(currentCell.x = cell.x - 1; currentCell.x <= cell.x +1; ++currentCell.x)
	for(currentCell.y = cell.y - 1; currentCell.y <= cell.y +1; ++currentCell.y)
	{
		// Init PRNG
		uint cx = currentCell.x < 0 ? 4294967295u - abs(currentCell.x) : uint(currentCell.x);
		uint cy = currentCell.y < 0 ? 4294967295u - abs(currentCell.y) : uint(currentCell.y);
		seeding(morton(cx,cy));

		int impulseSpotID = int(floor(randomIn(firstSpotID+0.5,firstSpotID + nbSpot - 0.5)));
		//int impulseSpotID = int(floor(randomIn(firstSpotID+0.5,lastSpotID+0.5)));

		for(int imp = 0; imp < dist.responseSize.z; ++imp)
		{

			mat4 Rj = R_j(vec4(0,0,0,0));
			mat4 Sj = S_j(vec3(1.0), vec3(1.0));
			vec2 impulsePos = currentCell + dist.p_center.xy + randomIn(dist.p_min.xy,dist.p_max.xy);
			vec4 spotColor = gaussiansSpot((vec4(point - impulsePos,0,1) * Rj * Sj ).xyz , uint(impulseSpotID));
			res += randomIn(dist.w_min,dist.w_max) * vec4(spotColor.rgb * spotColor.a, spotColor.a) ;
		}
	}
	res = res.a > 1.0 ? res / res.a : res; // Renormalize if >1
	//res.rgb = res.a > 0.0 ? res.rgb / res.a : res.rgb; // Recompute true color if < 1
	//res.rgb *= res.a;
	//res = res * spotWeight[spotID];
	//res.xyz *= (res.a * spotWeight[spotID]);
	//res.a *= spotWeight[spotID];
	
	return res;

}

vec4 NonUniformControlledRandomImpulseSpotNoise(vec2 p, int firstSpotID, int nbSpot, int distribID, sampler2D dataField ) {

	CachedDistribution dist = fromDistributionBuffer(distrib[distribID]);
	
	vec4 res = vec4(0.0f);
	vec2 point = p * dist.responseSize.xy;
	// Current evaluation cell
	ivec2 cell = ivec2(floor(point));
	
	vec2 realPoint  = p / 4.0f;

	float localVariation = 0.0;

	ivec2 currentCell;
	for(currentCell.x = cell.x - 1; currentCell.x <= cell.x +1; ++currentCell.x)
	for(currentCell.y = cell.y - 1; currentCell.y <= cell.y +1; ++currentCell.y)
	{
		// Init PRNG
		uint cx = currentCell.x < 0 ? 4294967295u - abs(currentCell.x) : uint(currentCell.x);
		uint cy = currentCell.y < 0 ? 4294967295u - abs(currentCell.y) : uint(currentCell.y);
		seeding(morton(cx,cy));

		for(int imp = 0; imp < dist.responseSize.z; ++imp)
		{
			vec2 impulsePos = currentCell + dist.p_center.xy + randomIn(dist.p_min.xy,dist.p_max.xy);
			vec4 data = vec4(1.0);
			data = texture2D(dataField , (impulsePos * 0.125f ) / dist.responseSize.xy  );

			int impulseSpotID = int(floor(randomIn(firstSpotID+0.5,firstSpotID + nbSpot - 0.5)));
			mat4 Rj = R_j(vec4(0,0,0,data.r * PIx2));

			mat4 Sj = S_j(  vec3(1.0) / (1.0 - data.g),vec3(1.0) / (1.0 - data.g) );
			
			vec4 spotColor = gaussiansSpot((vec4(point - impulsePos,0,1) * Rj * Sj ).xyz , uint(impulseSpotID));
			res += randomIn(dist.w_min,dist.w_max) * vec4(spotColor.rgb * spotColor.a, spotColor.a) ;
		}
	}
	res = res.a > 1.0 ? res / res.a : res; // Renormalize if >1
	//res.rgb = res.a > 0.0 ? res.rgb / res.a : res.rgb; // Recompute true color if < 1
	//res.rgb *= res.a;
	//res = res * spotWeight[spotID];
	//res.xyz *= (res.a * spotWeight[spotID]);
	//res.a *= spotWeight[spotID];
	
	return res;

}

vec4 NonUniformControlledRandomImpulseMultiSpotNoise(vec2 p, int firstSpotID, int nbSpot, int distribID, sampler2D dataField ) {

	CachedDistribution dist = fromDistributionBuffer(distrib[distribID]);
	
	vec4 res = vec4(0.0f);
	vec2 point = p * dist.responseSize.xy;
	// Current evaluation cell
	ivec2 cell = ivec2(floor(point));
	
	vec2 realPoint  = p / 4.0f;

	float localVariation = 0.0;

	ivec2 currentCell;
	for(currentCell.x = cell.x - 1; currentCell.x <= cell.x +1; ++currentCell.x)
	for(currentCell.y = cell.y - 1; currentCell.y <= cell.y +1; ++currentCell.y)
	{
		// Init PRNG
		uint cx = currentCell.x < 0 ? 4294967295u - abs(currentCell.x) : uint(currentCell.x);
		uint cy = currentCell.y < 0 ? 4294967295u - abs(currentCell.y) : uint(currentCell.y);
		seeding(morton(cx,cy));

		for(int imp = 0; imp < dist.responseSize.z; ++imp)
		{
			vec2 impulsePos = currentCell + dist.p_center.xy + randomIn(dist.p_min.xy,dist.p_max.xy);
			vec4 data = vec4(1.0);
			data = texture2D(dataField , (impulsePos * 0.125f ) / dist.responseSize.xy  );

			// using dataField as spot selector :
			float randomChoice = randomIn(0.0f,1.0f);
			int impulseSpotID = 0;
			if(randomChoice >= data.r) impulseSpotID = 1;

			mat4 Rj = R_j(vec4(0,0,0,0.0));
			mat4 Sj = S_j(  vec3(1.0), vec3(1.0) );
			
			vec4 spotColor = gaussiansSpot((vec4(point - impulsePos,0,1) * Rj * Sj ).xyz , uint(impulseSpotID));
			res += (spotWeight[0]) * randomIn(dist.w_min,dist.w_max) * vec4(spotColor.rgb * spotColor.a, spotColor.a) ;
		}
	}
	//res = res.a > 1.0 ? res / res.a : res; // Renormalize if >1
	//res.rgb = res.a > 0.0 ? res.rgb / res.a : res.rgb; // Recompute true color if < 1
	//res.rgb *= res.a;
	//res = res * spotWeight[spotID];
	//res.xyz *= (res.a * spotWeight[spotID]);
	//res.a *= spotWeight[spotID];
	
	return res;

}

// Version pour prez
vec4 LocallyControledSpotNoise(
	vec2 p, // Evaluated position in the texture space
	int firstSpotID, // ID of the First spot to use in the noise
	int nbSpot, // Number of different spot to use (spots used are in )
	int localDistribID, 
	sampler2D globalDensityField, 
	float densityFieldResolution) 
{

	CachedDistribution dist = fromDistributionBuffer(distrib[localDistribID]);
	
	vec4 res = vec4(0.0f);
	vec2 point = p * dist.responseSize.xy;
	// Current evaluation cell
	ivec2 cell = ivec2(floor(point));

	float localVariation = 0.0;

	ivec2 currentCell;
	for(currentCell.x = cell.x - 1; currentCell.x <= cell.x +1; ++currentCell.x)
	for(currentCell.y = cell.y - 1; currentCell.y <= cell.y +1; ++currentCell.y)
	{
		// Init PRNG
		uint cx = currentCell.x < 0 ? 
			4294967295u - abs(currentCell.x) : 
			uint(currentCell.x);
		uint cy = currentCell.y < 0 ? 
			4294967295u - abs(currentCell.y) : 
			uint(currentCell.y);
		
		seeding(morton(cx,cy));

		for(int imp = 0; imp < dist.responseSize.z; ++imp)
		{
			vec2 impulsePos = currentCell 
				+ dist.p_center.xy 
				+ randomIn(dist.p_min.xy,dist.p_max.xy);
			vec4 data = vec4(1.0);
			data = texture2D(
				globalDensityField , 
				impulsePos * densityFieldResolution / dist.responseSize.xy );

			// using dataField as spot selector :
			float densityTest = randomIn(0.1f,0.9f);
			int impulseSpotID = int(randomIn(firstSpotID,nbSpot));
			float validImpulse = densityTest < data.x ? 1.0f : 0.0f;
			
			mat4 Rj = R_j(vec4(0,0,0,0.0));
			mat4 Sj = S_j( vec3(1.0), vec3(1.0) );

			vec4 spotColor = gaussiansSpot(
				(vec4(point - impulsePos,0,1) * Rj * Sj ).xyz , 
				uint(impulseSpotID));

			res += validImpulse 
				* (spotWeight[0]) 
				* randomIn(dist.w_min,dist.w_max) 
				* vec4(spotColor.rgb * spotColor.a, spotColor.a) ;
		}
	}
	
	return res;

}

float LocallyControledSpotDistribution(
	vec2 p,
	int nbSpot, 
	int localDistribID, 
	sampler2D globalDensityField,
	float densityFieldResolution)
{
	CachedDistribution dist = fromDistributionBuffer(
		distrib[localDistribID]);
	
	float res = 0.0f;
	vec2 point = p * dist.responseSize.xy;
	// Current evaluation cell
	ivec2 cell = ivec2(floor(point));

	
	ivec2 currentCell;
	for(currentCell.x = cell.x - 1; currentCell.x <= cell.x +1; ++currentCell.x)
	for(currentCell.y = cell.y - 1; currentCell.y <= cell.y +1; ++currentCell.y)
	{
		// Init PRNG
		uint cx = currentCell.x < 0 ? 
			4294967295u - abs(currentCell.x) : 
			uint(currentCell.x);
		uint cy = currentCell.y < 0 ? 
			4294967295u - abs(currentCell.y) : 
			uint(currentCell.y);
		
		seeding(morton(cx,cy));

		for(int imp = 0; imp < dist.responseSize.z; ++imp)
		{
			vec2 impulsePos = currentCell 
				+ dist.p_center.xy 
				+ randomIn(dist.p_min.xy,dist.p_max.xy);
			vec4 data = vec4(1.0);
			//data = texture2D(
			//	globalDensityField, 
			//	impulsePos * densityFieldResolution  / dist.responseSize.xy);

			// using dataField as spot selector :
			float densityTest = randomIn(0.1f,0.9f);
			int impulseSpotID = int(randomIn(0,nbSpot));
			float validImpulse = densityTest < data.x ? 1.0f : 0.0f;
			
			mat4 Rj = R_j(vec4(0,0,0,0.0));
			mat4 Sj = S_j( vec3(1.0), vec3(1.0) );

			float d = length(point - impulsePos) < 0.1 ? 1.0 : 0.0;

			res += validImpulse * randomIn(dist.w_min,dist.w_max) * d;
		}
	}
	return 0.5f + 0.5f * clamp(res,-1.0f,1.0f);
	//return texture2D(globalDensityField, (point/ dist.responseSize.xy) * densityFieldResolution ).r;

}

vec4 LocallyControledSpotDistributionWithBackground(
	vec2 p,
	int nbSpot, 
	int localDistribID, 
	sampler2D globalDensityField,
	float densityFieldResolution)
{
	CachedDistribution dist = fromDistributionBuffer(
		distrib[localDistribID]);
	
	float res = 0.0f;
	vec2 point = p * dist.responseSize.xy;
	// Current evaluation cell
	ivec2 cell = ivec2(floor(point));

	
	ivec2 currentCell;
	for(currentCell.x = cell.x - 1; currentCell.x <= cell.x +1; ++currentCell.x)
	for(currentCell.y = cell.y - 1; currentCell.y <= cell.y +1; ++currentCell.y)
	{
		// Init PRNG
		uint cx = currentCell.x < 0 ? 
			4294967295u - abs(currentCell.x) : 
			uint(currentCell.x);
		uint cy = currentCell.y < 0 ? 
			4294967295u - abs(currentCell.y) : 
			uint(currentCell.y);
		
		seeding(morton(cx,cy));

		for(int imp = 0; imp < dist.responseSize.z; ++imp)
		{
			vec2 impulsePos = currentCell 
				+ dist.p_center.xy 
				+ randomIn(dist.p_min.xy,dist.p_max.xy);
			vec4 data = vec4(0.0);
			data = texture2D(
				globalDensityField, 
				impulsePos * densityFieldResolution  / dist.responseSize.xy);

			// using dataField as spot selector :
			float densityTest = randomIn(0.1f,0.9f);
			int impulseSpotID = int(randomIn(0,nbSpot));
			float validImpulse = densityTest < data.x ? 1.0f : 0.0f;
			
			mat4 Rj = R_j(vec4(0,0,0,0.0));
			mat4 Sj = S_j( vec3(1.0), vec3(1.0) );

			float d = length(point - impulsePos) < 0.1 ? 1.0 : 0.0;

			res += validImpulse * randomIn(dist.w_min,dist.w_max) * d;
		}
	}
	vec4 final = vec4(min(res,1.0));
	final.y = final.z = 0.0f;
	float tex = texture2D(
		globalDensityField, 
		(point / dist.responseSize.xy) * densityFieldResolution ).r;
	vec4 background = vec4(tex);
	return mix(background,final,final.a);

}


#endif

