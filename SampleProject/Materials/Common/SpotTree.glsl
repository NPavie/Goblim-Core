
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

#ifndef SPOTS_TREE_GLSL
#define SPOTS_TREE_GLSL

#extension GL_ARB_shading_language_include : enable
#include "/Materials/Common/Random/Random"
#line 20

struct GaussianParam
{
	mat4 V;
	vec4 magnitude; // (for memory alignement, only the x value is used)
};

/** @brief Gaussienne simple : G(X) = K * e^(-0.5 * X^T*V*X )
	@param evaluatedPosition 	Vecteur X (Position évaluée dans l'espace du noyau)
	@param param				Paramètres de la gaussienne (magnitude et matrice V représentant une isosurface quadratic)
*/
float gaussianFunc(in vec3 evaluatedPosition, in GaussianParam param)
{
	return clamp(
		param.magnitude.x * exp(-0.5 * dot(vec4(evaluatedPosition, 1.0), param.V * vec4(evaluatedPosition, 1.0))),
		-1.0f,1.0f);
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
		scale.x * vec4((c.z * c.y), (s.z * c.y), -s.y, 0.0 ),
		scale.y * vec4((c.z * s.y * s.x - s.z * c.x) , (s.z * s.y * s.x + c.z * c.x ), (c.y * s.x), 0.0 ),
		scale.z * vec4((c.z * s.y * c.x + s.z * s.x) , (s.z * s.y * c.x - c.z * s.x ), (c.y * c.x), 0.0),
		vec4(shift,1.0));
	MRS = inverse(MRS);
	return transpose(MRS) * MRS;
	// distance to the surface ellipse surface is given by X^t * Q * X which is equivalent to the dot product dot(X, Q*X)
}

struct HarmonicParam
{
	vec4 thetaPhiFrequency;
};

// FLAGUNSURE
float harmonicFunc(in vec3 evaluatedPosition, in HarmonicParam param)
{
	return clamp(
		cos(PIx2 * param.f_sc.x * (evaluatedPosition.x * cos(param.f_sc.y) * sin(param.f_sc.z) + evaluatedPosition.y * sin(param.f_sc.y) * sin(param.f_sc.z) + evaluatedPosition.z * cos(param.f_sc.z) )),
		-1.0f, 1.0f);
}


float harmonic2DFunc(in vec3 evaluatedPosition, in HarmonicParam param)
{
	return cos(PIx2 * param.f_sc.x * (evaluatedPosition.x * cos(param.f_sc.y) + evaluatedPosition.y * sin(param.f_sc.y) ));
}


struct ConstantParam
{
	float constantValue;
};

// Inline if possible
float constantFunc(in vec3 evaluatedPosition, in ConstantParam param)
{
	return param.constantValue;
}


struct DistribParam
{
	float param[24];
};

// Test
struct LocalDistribParam
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

LocalDistribParam fromDistribParam(DistribParam dist)
{
	LocalDistribParam temp;
	temp.responseSize = vec3(dist.param[0],dist.param[1],dist.param[2]);
	temp.w_min = dist.param[3];
	temp.w_max = dist.param[4];
	temp.p_min = vec3(dist.param[5],dist.param[6],dist.param[7]);
	temp.p_center = vec3(dist.param[8],dist.param[9],dist.param[10]);
	temp.p_max = vec3(dist.param[11],dist.param[12],dist.param[13]);
	temp.rotate_min_max = vec4(dist.param[14],dist.param[15],dist.param[16],dist.param[17]);
	temp.scale_min = vec3(dist.param[18],dist.param[19],dist.param[20]);
	temp.scale_max = vec3(dist.param[21],dist.param[22],dist.param[23]);
	return temp;
}


// parameters buffers
layout (std430,binding=3) buffer gaussianDataBuffer
{
	GaussianParam gaussian[];
};

layout (std430,binding=4) buffer harmonicDataBuffer
{
	HarmonicParam harmonic[];
};

layout (std430,binding=5) buffer constantDataBuffer
{
	ConstantParam constant[];
};

layout (std430,binding=6) buffer distribDataBuffer
{
	// distrib : response.xyz, weigtRange.xy, posMinCenterMax[3][3], rotationsMinMax.xyzw, scaleMin.xyz, scaleMax.xyz
	DistribParam distrib[];
};

// Data buffer
layout (std430,binding=7) buffer spotDataBuffer
{
	int spotData[];				// spotData : 	nbFunction_S0, F_1[Operand_1,typeFunction_1,paramId_1], F_2[], nbFunction_S1...
};

layout (std430,binding=8) buffer spotsTreeNodeDataBuffer
{
	int spotsTreeData[];
};

// Index buffers
layout (std430,binding=9) buffer spotIndexBuffer
{
	int spotIndex[];
};

layout (std430,binding=10) buffer spotsTreeNodeIndexBuffer
{
	int spotsTreeNodeIndex[];
};

layout (std430,binding=11) buffer sizeOfArraysBuffer
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

// Evaluate a node response
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
				func_eval = harmonic2DFunc(p,harmonic[param]);
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

float w_j(vec3 posToProfileCenter, int profileSpotId, float w_min, float w_max)
{
	float weightRange = w_max - w_min;
	// profile in [0,1] => [-1,1]
	float weightCursorMovement = ( spotResponse(posToProfileCenter, profileSpotId) - 0.5 ) * 2.0f;

	// A weight profile move the distribution range : wp < 0.5 move the cursor toward lower weight and wp > 0.5 move the cursor toward higher weight
	// if the profile is equal to 0.5, the weight distribution stays unchanged in [w_min, w_max]
	// Wp = 0 => cursorMax move to w_min , Wp = 1 => cursorMin move to w_max

	float cursor_min = max(w_min, w_min + (weightCursorMovement*weightRange));
	float cursor_max = min(w_max, w_max + (weightCursorMovement*weightRange));

	return cursor_min + (cursor_max - cursor_min) * random();

}

mat4 R_j(vec4 rotationMinMax)
{

	vec2 randomRotate = rotationMinMax.xy + (rotationMinMax.zw - rotationMinMax.xy) * random();
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


#define MAXLVL 3

struct Node {
	int firstData;
	LocalDistribParam dist;
	int distribSpotID;
	int spotID;
	int nbSons;
};


// retrieve node data from global memory
Node getNode(int nodeIndex)
{
	Node treeNode;
	treeNode.firstData = spotsTreeNodeIndex[nodeIndex];
	treeNode.dist = fromDistribParam( distrib[spotsTreeData[treeNode.firstData]] );
	treeNode.distribSpotID = spotsTreeData[treeNode.firstData + 1];
	treeNode.spotID = spotsTreeData[treeNode.firstData + 2];
	treeNode.nbSons = spotsTreeData[treeNode.firstData + 3];

	return treeNode;
}

int getSonID(Node root, int numFils)
{
	return spotsTreeData[root.firstData + 3 + numFils];
}

struct Impulsion {
	vec3  pos;
	mat4  rotation;
	mat4  scale;
};

Impulsion createImpulseOf(Node n, ivec2 inCell)
{
	Impulsion i;
	i.pos.xy = inCell + (n.dist.p_center.xy +  randomIn( n.dist.p_min.xy , n.dist.p_max.xy ));
	i.pos.z = 0;
	i.rotation = R_j(n.dist.rotate_min_max);
	i.scale = S_j(n.dist.scale_min, n.dist.scale_max) ;
	return i;

}

// Default impulse random position in cell, random rotation, no rescale
Impulsion createDefaultImpulseIn(ivec2 cell)
{
	Impulsion i;
	i.pos.xy = cell + random();
	i.pos.z = 0;
	i.rotation = R_j(vec4(0,0,0,PI));
	i.scale = S_j(vec3(1.0),vec3(1.0));
	return i;

}

Impulsion create2DImpulseIn(
	ivec2 cell, 
	vec2 posMinInCell, 
	vec2 posMaxInCell, 
	vec2 thetaPhiMin,
	vec2 thetaPhiMax,
	vec3 scaleMin,
	vec3 scaleMax)
{
	Impulsion i;
	i.pos.xy = cell + randomIn(posMinInCell,posMaxInCell);
	i.pos.z = 0;
	i.rotation = R_j(vec4(thetaPhiMin,thetaPhiMax));
	i.scale = S_j(scaleMin,scaleMax);
	return i;

}

Impulsion createImpulseOf(Node n, ivec3 inCell)
{
	Impulsion i;
	i.pos = inCell + (n.dist.p_center +  randomIn( n.dist.p_min , n.dist.p_max ));
	i.rotation = R_j(n.dist.rotate_min_max);
	i.scale = S_j(n.dist.scale_min, n.dist.scale_max) ;
	return i;

}

float sumOfWeight = 0.0;

float evalImpulseAt(vec2 evalpos, Impulsion i, Node n )
{
	vec3 pos_pos_j = ( i.rotation * i.scale * vec4( evalpos - i.pos.xy , 0,1) ).xyz;
	//float weight = w_j(fract(vec3(i.pos.xy,0)), n.distribSpotID, n.dist.w_min, n.dist.w_max);
	//sumOfWeight += weight;
	if(length(pos_pos_j) > 2.0) return 0.0;

	float weight = randomIn(n.dist.w_min, n.dist.w_max);
	return clamp( weight * spotResponse(pos_pos_j,n.spotID),-1.0,1.0);
}

float evalImpulseAt(vec3 evalpos, Impulsion i, Node n )
{
	vec3 pos_pos_j = ( i.rotation * i.scale * vec4( evalpos - i.pos,1) ).xyz;

	return clamp(w_j(fract(i.pos), n.distribSpotID, n.dist.w_min, n.dist.w_max) * spotResponse(pos_pos_j,n.spotID),-1.0,1.0);
}



int impulseDensity = 1;

float evaluateNoiseNode(vec2 surfacePoint, float zoom, int nodeID)
{
	float res = 0;

	Node noeudEvaluer = getNode(nodeID);

	Impulsion current;

	vec2 evalPos = (surfacePoint / zoom) / noeudEvaluer.dist.responseSize.xy;
	ivec2 currentCell = ivec2(floor(evalPos));


	for(int yi = -1; yi <= 1; yi++)
	for(int xi = -1; xi <= 1; xi++)
	{
		float cellRes = 0.0;
		ivec2 evalCell = currentCell + ivec2(xi,yi);
		uint cx = (evalCell.x) < 0 ? 4294967295u - abs(evalCell.x) : uint(evalCell.x);
		uint cy = evalCell.y < 0 ? 4294967295u - abs(evalCell.y) : uint(evalCell.y);
		seeding(morton(cx,cy));


		for(int i = 0; i < impulseDensity; i++) // impulse density
		{
			current = createImpulseOf(noeudEvaluer, evalCell);
			cellRes += evalImpulseAt(evalPos,current,noeudEvaluer);
		}
		res += cellRes;

	}
	//return clamp(res,-1.0,1.0);
	// Renormalisation du resultat : si abs(sumWeight / impulseDensity) > (moyenne théorique des poids)
	return res;// / sqrt(impulseDensity);
}


float evaluateNoiseFBO(vec2 surfacePoint, float zoom, sampler2D previousLevelSampler, int evaluatedNodeID)
{
	uint stateBackup;
	float evalLastLevel = 1.0;

	float resNiveau = 0;
	Node noeudEvaluer = getNode(evaluatedNodeID);

	Impulsion current;

	vec2 evalPos = (surfacePoint / zoom) / noeudEvaluer.dist.responseSize.xy;
	ivec2 currentCell = ivec2(floor(evalPos));

	for(int yi = -1; yi <= 1; yi++)
	for(int xi = -1; xi <= 1; xi++)
	{
		ivec2 evalCell = currentCell + ivec2(xi,yi);
		uint cx = (evalCell.x) < 0 ? 4294967295u - abs(evalCell.x) : uint(evalCell.x);
		uint cy = evalCell.y < 0 ? 4294967295u - abs(evalCell.y) : uint(evalCell.y);
		seeding(morton(cx,cy));
		for(int i = 0; i < impulseDensity; i++) // impulse density line 379
		{
			current = createImpulseOf(noeudEvaluer, evalCell);
			//evaluateNoiseNode(current.pos.xy * noeudEvaluer.dist.responseSize.xy,n-1)
			resNiveau += texture(previousLevelSampler, current.pos.xy * zoom * noeudEvaluer.dist.responseSize.xy).x  * evalImpulseAt(evalPos,current,noeudEvaluer);
		}

	}



	//return clamp(resNiveau,-1.0,1.0);
	return resNiveau;// / sqrt(impulseDensity);

}



vec3 posInImpulseSpace(vec3 evalpos, Impulsion i)
{
	return ( i.rotation * i.scale * vec4( evalpos - i.pos,1.0) ).xyz;
}

float nonRecursiveTreeEval(vec2 surfacePoint)
{


	int nbFilsATraiter[MAXLVL];
	Impulsion impulseAtLvl[MAXLVL];
	float res[MAXLVL];
	int nbImpulseToCreate[MAXLVL];
	int nbImpulseCreated[MAXLVL];
	ivec2 cellEvalAtLvl[MAXLVL];
	ivec2 cellModifier[MAXLVL];
	uint prngState[MAXLVL];

	vec2 evalPosAtLvl[MAXLVL];

	int niveauCourant = 0;
	int nodeIdStack[MAXLVL];

	for(int i = 0; i < MAXLVL ; i++)
	{
		cellModifier[i] = ivec2(-2,-2);
		nodeIdStack[i] = -1;
	}

	int impulseDensity = 32;

	nbImpulseToCreate[niveauCourant] = impulseDensity * 9;

	nodeIdStack[niveauCourant] = 0;

	Node noeudCourant = getNode(nodeIdStack[niveauCourant]);
	res[niveauCourant] = 0;
	nbFilsATraiter[niveauCourant] = noeudCourant.nbSons;

	evalPosAtLvl[niveauCourant] = surfacePoint / noeudCourant.dist.responseSize.xy;
	cellEvalAtLvl[niveauCourant] = ivec2(floor(evalPosAtLvl[niveauCourant]));

	while(nbImpulseToCreate[0] > 0)
	{
		// PRNG initialize : cell commutater
		if(nbImpulseToCreate[niveauCourant] % impulseDensity == 0)
		{	// If the number of impulse to create is a multiple of the density of a cell, we change the cell X
			cellModifier[niveauCourant].x += 1;
			if(nbImpulseToCreate[niveauCourant] % (impulseDensity*3) == 0)
			{	// If the number of impulse to create is a multiple of the density of a whole line, we change the cell X and Y
				cellModifier[niveauCourant].x = -1;
				cellModifier[niveauCourant].y += 1;
			}
			// Compute the cell according the current number of impulse
			ivec2 evalCell = cellEvalAtLvl[niveauCourant] + cellModifier[niveauCourant];
			uint cx = (evalCell.x) < 0 ? 4294967295u - abs(evalCell.x) : uint(evalCell.x);
			uint cy = evalCell.y < 0 ? 4294967295u - abs(evalCell.y) : uint(evalCell.y);
			// initialize the prng
			seeding(morton(cx,cy));

		}
		// Generating an impulse (pos,scale,rotate)
		impulseAtLvl[niveauCourant] = createImpulseOf(noeudCourant, cellEvalAtLvl[niveauCourant] + cellModifier[niveauCourant]);
		// Decresing the remaining number of impulse to create
		--nbImpulseToCreate[niveauCourant];
 		// If for this impulse there is sons to compute (within the limit of lvl passed through)
		if(nbFilsATraiter[niveauCourant] > 0 && niveauCourant < MAXLVL)
		{
			// Backing current prng state
			prngState[niveauCourant] = getState();
			// Switching to next level
			++niveauCourant;
			// Storing the node ID of the current level
			nodeIdStack[niveauCourant] = getSonID(noeudCourant, noeudCourant.nbSons - nbFilsATraiter[niveauCourant]);
			// retrieving the node data
			noeudCourant = getNode(nodeIdStack[niveauCourant]);
			// initialis
			res[niveauCourant] = 0;
			nbImpulseToCreate[niveauCourant] = impulseDensity * 9;

			evalPosAtLvl[niveauCourant] = evalPosAtLvl[niveauCourant-1] / noeudCourant.dist.responseSize.xy;
			cellEvalAtLvl[niveauCourant] = ivec2(floor(evalPosAtLvl[niveauCourant]));

			if(noeudCourant.nbSons > 0)
			{
				nbFilsATraiter[niveauCourant] = noeudCourant.nbSons;
			}

		}
		else
		{
			float weight_up = 1.0;
			if(niveauCourant > 0)
			{
				// Compute weight of new impulse by evaluating its pos in the reference impulse
				//vec2 impulsePosInRefNode = impulseAtLvl[niveauCourant].pos.xy * noeudCourant.dist.responseSize.xy;
				//vec2 posInImpulseSpace = posInImpulseSpace(impulsePosInRefNode, impulseAtLvl[niveauCourant-1] );
				//weight_up = w_j(posInImpulseSpace, , float w_min, float w_max)

			}
			//
			res[niveauCourant] += weight_up * evalImpulseAt(evalPosAtLvl[niveauCourant], impulseAtLvl[niveauCourant], noeudCourant );

		}

		// finally, if we have finished the impulse distribution
		if(nbImpulseToCreate[niveauCourant] == 0 && niveauCourant > 0)
		{
			// Back to upper level
			--niveauCourant;
			// retrieve the last node
			noeudCourant = getNode(nodeIdStack[niveauCourant]);
			// reset PRNG state
			setState(prngState[niveauCourant]);
			// Backing results of the subnode distribution in the current level
			res[niveauCourant] += clamp(res[niveauCourant+1],-1.0f,1.0f);
			// decrementing the number of son to compute
			--nbFilsATraiter[niveauCourant];
		}

	}

	return 0.5 + 0.5 * res[0];
}


float evaluateNoiseTest_2(vec2 surfacePoint)
{
	float res = 0;

	uint stateBackup;
	float evalLastLevel = 1.0;
	for(int n = 0; n < spotsTreeNodeIndexArraySize ; n++)
	{
		float resNiveau = 0;
		Node noeudEvaluer = getNode(n);

		Impulsion current;

		vec2 evalPos = surfacePoint / noeudEvaluer.dist.responseSize.xy;
		ivec2 currentCell = ivec2(floor(evalPos));

		for(int yi = -1; yi <= 1; yi++)
		for(int xi = -1; xi <= 1; xi++)
		{
			ivec2 evalCell = currentCell + ivec2(xi,yi);
			uint cx = (evalCell.x) < 0 ? 4294967295u - abs(evalCell.x) : uint(evalCell.x);
			uint cy = evalCell.y < 0 ? 4294967295u - abs(evalCell.y) : uint(evalCell.y);
			seeding(morton(cx,cy));
			for(int i = 0; i < 8; i++) // impulse density
			{

				current = createImpulseOf(noeudEvaluer, evalCell);
				stateBackup = getState();
				//if(n == 0)
					resNiveau += evalLastLevel * evalImpulseAt(evalPos,current,noeudEvaluer);
				//else res += evaluateNoiseNode(current.pos.xy * noeudEvaluer.dist.responseSize.xy,n-1) * evalImpulseAt(evalPos,current,noeudEvaluer);
				setState(stateBackup);
			}

		}
		res += resNiveau;
		evalLastLevel = resNiveau;
	}

	return clamp(res,-1.0,1.0);

}


// flagcurrent
float hardNoiseTest(vec2 surfacePoint, float zoom, int evaluatedNodeID)
{
	uint stateBackup;
	float evalLastLevel = 1.0;

	float resNiveau = 0;
	Node noeudEvaluer = getNode(evaluatedNodeID);

	Impulsion current;

	vec2 evalPos = (surfacePoint / zoom) * 10.0;// / noeudEvaluer.dist.responseSize.xy;
	ivec2 currentCell = ivec2(floor(evalPos));

	for(int y_i = -1; y_i < 2; y_i++)
	for(int x_i = -1; x_i < 2; x_i++)
	{
		ivec2 evalCell = currentCell + ivec2(x_i,y_i);
		// ensure that negativity is recognize as MAXVAL - value
		uint cx = (evalCell.x) < 0 ? 4294967295u - abs(evalCell.x) : uint(evalCell.x);
		uint cy = evalCell.y < 0 ? 4294967295u - abs(evalCell.y) : uint(evalCell.y);
		seeding(morton(cx,cy));
		// Create a single impulsion as reference distribution center
		//current = createDefaultImpulseIn(evalCell); 	// Pour modifier la distrib, il faut aller ligne 337
		
		for(int i = 0; i < 32; i++) // impulse density line 379
		{
			current = create2DImpulseIn(
					evalCell, 
					vec2(0.0), 
					vec2(0.0), 
					vec2(0.0),
					vec2(0,0),
					vec3(1.0),
					vec3(1.0)
					);
		// Test - one impulse, several randomly distributed gaussian (sub impulses)
			vec2 pointInImpulse = posInImpulseSpace(vec3(evalPos,0.0),current).xy;
			// distribute randomly a set of elliptical gaussians
			GaussianParam gauss;
			// Note : damn i dont remember where i computed our gaussian moment but its 4.29
			gauss.V = ellipseIsoSurfaceMatrix(
						vec3(randomVec2(),0.0), // random pos in -0.5,0.5
						vec3(noeudEvaluer.dist.responseSize.xy,1.0) * vec3(1.0,1.0,1.0) , 		// fixed gaussian size 
						vec3(0.0,0.0, 0.0)		// random orientation in 0,0 : 0,Pi
						);
			gauss.magnitude.x = 1.0;
			resNiveau += randomIn(-1.0,1.0) * gaussianFunc(vec3(pointInImpulse,0), gauss);

		}

	}

	//return clamp(resNiveau,-1.0,1.0);
	return 0.5 + 0.5 * clamp(resNiveau,-1.0,1.0);// / sqrt(impulseDensity);

}


/*
// basic test function (root node considred to have no sons) - WORKIIING time to pass to derecusive test
float surface_noise_testSimpleSpot(vec2 surfacePoint)
{
	float res = 0;

	// retrieve node ID pf first data
	int nodeFirstDataID = spotsTreeNodeIndex[0];

	// Retrieve IDs of distribParam, local and global profile, and nbSons
	int distribID = spotsTreeData[nodeFirstDataID];
	int localDistribSpotID = spotsTreeData[nodeFirstDataID + 1];
	int nodeResponseID = spotsTreeData[nodeFirstDataID + 2];
	int nbSons = spotsTreeData[nodeFirstDataID + 3];


	LocalDistribParam localDistParam = fromDistribParam( distrib[distribID] );


	vec2 evalPos = surfacePoint / localDistParam.responseSize.xy;
	ivec2 currentCell = ivec2(floor(evalPos));

	for(int yi = -1; yi <= 1; yi++)
	for(int xi = -1; xi <= 1; xi++)
	{
		ivec2 evalCell = currentCell + ivec2(xi,yi);
		uint cx = (evalCell.x) < 0 ? 4294967295u - abs(evalCell.x) : uint(evalCell.x);
		uint cy = evalCell.y < 0 ? 4294967295u - abs(evalCell.y) : uint(evalCell.y);
		seeding(morton(cx,cy));

		for(int i = 0; i < 8; i++) // impulse density
		{

			vec2 pos_j = localDistParam.p_center.xy
			+  randomIn( localDistParam.p_min.xy , localDistParam.p_max.xy ); // normalement distrib pos_min et max

			vec3 pos_pos_j = ( R_j(localDistParam.rotate_min_max)
				* S_j(localDistParam.scale_min, localDistParam.scale_max)
				* vec4( evalPos - (evalCell + pos_j) , 0,1) ).xyz; // normalement R_j * S_j * (evalPos - pos_j)

			res += w_j(pos_pos_j, localDistribSpotID, localDistParam.w_min, localDistParam.w_max) * spotResponse(pos_pos_j,nodeResponseID);
			//
		}

	}

	return 0.5 + 0.5 * res;
}



/*
float evaluateNoiseTest(vec2 surfacePoint)
{
	float res = 0;

	uint stateBackup;
	for(int n = 0; n < spotsTreeNodeIndexArraySize ; n++)
	{
		Node noeudEvaluer = getNode(n);

		Impulsion current;

		vec2 evalPos = surfacePoint / noeudEvaluer.dist.responseSize.xy;
		ivec2 currentCell = ivec2(floor(evalPos));

		for(int yi = -1; yi <= 1; yi++)
		for(int xi = -1; xi <= 1; xi++)
		{
			ivec2 evalCell = currentCell + ivec2(xi,yi);
			uint cx = (evalCell.x) < 0 ? 4294967295u - abs(evalCell.x) : uint(evalCell.x);
			uint cy = evalCell.y < 0 ? 4294967295u - abs(evalCell.y) : uint(evalCell.y);
			seeding(morton(cx,cy));
			for(int i = 0; i < 8; i++) // impulse density
			{
				current = createImpulseOf(noeudEvaluer, evalCell);
				stateBackup = getState();
				if(n == 0) res += evalImpulseAt(evalPos,current,noeudEvaluer);
				else res += evaluateNoiseNode(current.pos.xy * noeudEvaluer.dist.responseSize.xy,n-1) * evalImpulseAt(evalPos,current,noeudEvaluer);
				setState(stateBackup);
			}

		}
	}

	return clamp(res,-1.0,1.0);

}
*/


/*
	Algo de dérécursion
	niveauCourant = 0

	NoeudCourant = root
	pour chaque niveau nbFilsAtraité[niveau] = -1

	nbFilsAtraité[niveauCourant] = root.nbFils 	// on récupère le nombre de fils à traité
	result[niveauCourant]
	nbImpulsesToEval[0] = densité * nbCases		// au départ on a N impulse du noeud root à traité
	res[0] = 0
	Tant que (nbImpulsesToEval[0] > 0) 			// tant qu'il reste des impulses du noeud root à traité
	{
		impulseEval[niveauCourant] = générer une impulse pour noeudCourant
		nbImpulseToEval[niveauCourant]--;		// décrémenté le nombre d'impulse à générer pour ce niveau

		si nbFilsAtraité[niveauCourant] > 0 && niveauCourant < niveauMax 									// s'il y a des fils à évalué
			noeudCourant = noeudCourant.fils[noeudCourant.nbFils - nbFilsAtraité[niveauCourant]] // le prochain fils à évalué devient le noeud courant
			niveauCourant++														// on change de niveau
			res[niveauCourant] = 0;
			nbImpulseToEval[niveauCourant] = densité du prochain fils * nbCases	// on initialise le nombre d'impulsion à générer pour ce fils
			si noeudCourant.nbFils > 0 											// Si le nouveau noeud n'est pas terminal
				 nbFilsAtraité[niveauCourant] = noeudCourant.nbFils				// On change le nombre de fils à traité pour le nouveau noeud
		sinon 	// s'il n'y a pas ou plus de fils à traité (noeud terminal ou tout les fils évalué)
			poidsPere = niveauCourant-1 >= 0 ? pondération(pos par rapport impulseCourante[niveauCourant-1]) : 1 	// Pondération en fonction de la réponse du noeud parent s'il existe
			res[niveauCourant] += réponse de impulseEval[niveauCourant] * poidsPere
			// Impulse

		Si nbImpulseToEval[niveauCourant] == 0 				//si on a traité toute les impulse du niveau courant
			niveauCourant--									// on revient au niveau supérieur
			res[niveauCourant] += res[niveauCourant+1]		// On ajoute le résultat du niveau précédent au niveau courant
			si niveauCourant >= 0 								// si l'on est toujours dans l'arbre
				nbFilsATraité[niveauCourant]--					// on décrémente le nombre de fils restant à traité





		Fin sinon


	}



----------- Backup
float cubeCell(ivec3 c, vec3 p, int impulseDensity, int evalSpotID, int distribID, int localDistribSpotID, int globalDistribID, DistribParam dist)
{
	uint cx = c.x < 0 ? 4294967295u - abs(c.x) : uint(c.x);
	uint cy = c.y < 0 ? 4294967295u - abs(c.y) : uint(c.x);
	uint cz = c.z < 0 ? 4294967295u - abs(c.z) : uint(c.x);
	float res = 0.0;
	seeding(morton(cx,cy,cz));
	for(int i = 0; i < impulseDensity; i++)
	{
		// random distrib
		vec3 pos_j = randomVec3(); // normalement distrib pos_min et max

		vec3 pos_pos_j = p - (c + pos_j); // normalement R_j * S_j * (evalPos - pos_j)
		if( dot(pos_pos_j,pos_pos_j) < 1.0)	res += randomIn(-1.0,1.0) * spotResponse(pos_pos_j,evalSpotID);
	}

	return  clamp(res,-1.0,1.0);
}

float node_lvl_i_0(vec3 p)
{
	float res = 0;
	// simple test d'évaluation du noeur root
	int nodeIdInIndex = 0;

	// retrieve node ID pf first data
	int nodeFirstDataID = spotsTreeNodeIndex[nodeIdInIndex];

	// Retrieve IDs of distribParam, local and global profile, and nbSons
	int distribID = spotsTreeData[nodeFirstDataID];
	int localDistribSpotId = spotsTreeData[nodeFirstDataID+1];
	int globalDistribSpotId = spotsTreeData[nodeFirstDataID + 2];
	int nbSons = spotsTreeData[nodeFirstDataID + 3];

	// size of node response
	vec3 nodeResponse = vec3(distrib[distribID].param[0],distrib[distribID].param[1],distrib[distribID].param[2]);
	vec3 evalPos = p / nodeResponse;

	if(nbSons <= 0)
	{
		int spotIndexID = spotsTreeData[nodeFirstDataID + 4];
		ivec3 currentCell = ivec3(floor(evalPos));
		for(int x_i = -1; x_i <= 1; ++x_i)
		{
			for(int y_i = -1; y_i <= 1; ++y_i)
			{
				for(int z_i = -1; z_i <= 1; ++z_i)
				{
					res += cubeCell(currentCell+ivec3(x_i,y_i,z_i),evalPos, 32, spotIndexID, distribID, localDistribSpotId,globalDistribSpotId,distrib[distribID]);
				}
			}
		}

	}
	else
	{
		res = 0.0;
	}

	return res;
}



float squareCell(ivec2 c, vec2 p, int impulseDensity, int evalSpotID, int localDistribSpotID, DistribParam dist)
{
	uint cx = c.x < 0 ? 4294967295u - abs(c.x) : uint(c.x);
	uint cy = c.y < 0 ? 4294967295u - abs(c.y) : uint(c.y);
	float res = 0.0;
	seeding(morton(cx,cy));

	vec3 scaleMin = vec3(dist.param[18],dist.param[19],dist.param[20]);
	vec3 scaleMax = vec3(dist.param[21],dist.param[22],dist.param[23]);
	vec4 rotationMinMax = vec4(dist.param[14],dist.param[15],dist.param[16],dist.param[17]);
	float w_min = dist.param[3];
	float w_max = dist.param[4];

	for(int i = 0; i < impulseDensity; i++)
	{
		// random distrib (mais controler quand meme)
		vec2 pos_j = vec2(dist.param[8],dist.param[9]) +  randomIn( vec2(dist.param[5],dist.param[6]) , vec2(dist.param[11],dist.param[12]) ); // normalement distrib pos_min et max

		vec3 pos_pos_j = ( R_j(rotationMinMax)
			* S_j(scaleMin, scaleMax)
			* vec4( p - (c + pos_j) , 0,1) ).xyz; // normalement R_j * S_j * (evalPos - pos_j)

		res += w_j(pos_pos_j, localDistribSpotID, w_min, w_max) * spotResponse(pos_pos_j,evalSpotID);
	}

	return  clamp(res,-1.0,1.0);
}

*/

#endif //SPOTS_TREE_GLSL
