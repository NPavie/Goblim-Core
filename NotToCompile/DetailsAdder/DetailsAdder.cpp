
#include "DetailsAdder.h"
//using namespace glm;


float clamp(float x, float min, float max)
{
	if (x < min) x = min;
	if (x > max) x = max;
	return x;
}

int clamp(int x, int min, int max)
{
	if (x < min) x = min;
	if (x > max) x = max;
	return x;
}

// dot2 = norme au carré d'un vecteur
float dot2(glm::vec3 v) { return glm::dot(v, v); }
float dot2(glm::vec2 v) { return glm::dot(v, v); }

glm::vec3 clamp(glm::vec3 testedVec, glm::vec3 min_v, glm::vec3 max_v)
{
	for (int i = 0; i < 3; i++)
	{
		if (testedVec[i] < min_v[i]) testedVec[i] = min_v[i];
		if (testedVec[i] > max_v[i]) testedVec[i] = max_v[i];
	}
	return testedVec;
}

// signed distance to a 2D triangle - IQ
float sdTriangle(glm::vec2 p0, glm::vec2 p1, glm::vec2 p2, glm::vec2 p)
{
	// edges
	glm::vec2 e0 = p1 - p0;
	glm::vec2 e1 = p2 - p1;
	glm::vec2 e2 = p0 - p2;

	// Distance of point to the
	glm::vec2 v0 = p - p0;
	glm::vec2 v1 = p - p1;
	glm::vec2 v2 = p - p2;
		
	glm::vec2 pq0 = v0 - e0* clamp(glm::dot(v0, e0) / glm::dot(e0, e0), 0.0, 1.0);
	glm::vec2 pq1 = v1 - e1* clamp(glm::dot(v1, e1) / glm::dot(e1, e1), 0.0, 1.0);
	glm::vec2 pq2 = v2 - e2* clamp(glm::dot(v2, e2) / glm::dot(e2, e2), 0.0, 1.0);

	// cross products
	glm::vec2 d = glm::min(glm::min(glm::vec2(glm::dot(pq0, pq0), v0.x*e0.y - v0.y*e0.x),
					glm::vec2(glm::dot(pq1, pq1), v1.x*e1.y - v1.y*e1.x)),
					glm::vec2(glm::dot(pq2, pq2), v2.x*e2.y - v2.y*e2.x));

	return -sqrt(d.x)*glm::sign(d.y);
}



float udTriangle(glm::vec3 v1, glm::vec3 v2, glm::vec3 v3, glm::vec3 p)
{
	glm::vec3 v21 = v2 - v1; glm::vec3 p1 = p - v1;
	glm::vec3 v32 = v3 - v2; glm::vec3 p2 = p - v2;
	glm::vec3 v13 = v1 - v3; glm::vec3 p3 = p - v3;
	glm::vec3 nor = glm::cross(v21, v13);

	// distance non signé
	return sqrt((glm::sign(glm::dot(glm::cross(v21, nor), p1)) +
		glm::sign(glm::dot(glm::cross(v32, nor), p2)) +
		glm::sign(glm::dot(glm::cross(v13, nor), p3))<2.0)
		?
		glm::min(glm::min(
		dot2(v21*clamp(glm::dot(v21, p1) / dot2(v21), 0.0, 1.0) - p1),
		dot2(v32*clamp(glm::dot(v32, p2) / dot2(v32), 0.0, 1.0) - p2)),
		dot2(v13*clamp(glm::dot(v13, p3) / dot2(v13), 0.0, 1.0) - p3))
		:
		glm::dot(nor, p1)*glm::dot(nor, p1) / dot2(nor));
}


// code form real-time collision detection - ça marche !
glm::vec3 triangleClosestPoint(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 & barycentriqueCoords)
{
	// Check if P in vertex region outside A
	glm::vec3 ab = b - a;
	glm::vec3 ac = c - a;
	glm::vec3 ap = p - a;
	float d1 = glm::dot(ab, ap);
	float d2 = glm::dot(ac, ap);
	if (d1 <= 0.0f && d2 <= 0.0f)
	{
		barycentriqueCoords = glm::vec3(1, 0, 0);
		return a; // barycentric coordinates (1,0,0)
	}
	
	// Check if P in vertex region outside B
	glm::vec3 bp = p - b;
	float d3 = glm::dot(ab, bp);
	float d4 = glm::dot(ac, bp);
	if (d3 >= 0.0f && d4 <= d3)
	{ 
		barycentriqueCoords = glm::vec3(0, 1, 0);
		return b; // barycentric coordinates (0,1,0)
	}
	
	// Check if P in edge region of AB, if so return projection of P onto AB
	float vc = d1*d4 - d3*d2;
	if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) 
	{
		float v = d1 / (d1 - d3);
		barycentriqueCoords = glm::vec3(1.0 - v, v, 0);
		return a + v * ab; // barycentric coordinates (1-v,v,0)
	}
	// Check if P in vertex region outside C
	glm::vec3 cp = p - c;
	float d5 = glm::dot(ab, cp);
	float d6 = glm::dot(ac, cp);
	if (d6 >= 0.0f && d5 <= d6)
	{
		barycentriqueCoords = glm::vec3(0, 0, 1);
		return c; // barycentric coordinates (0,0,1)
	}

	// Check if P in edge region of AC, if so return projection of P onto AC
	float vb = d5*d2 - d1*d6;
	if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) 
	{
		float w = d2 / (d2 - d6);
		barycentriqueCoords = glm::vec3(1.0-w, 0, w);
		return a + w * ac; // barycentric coordinates (1-w,0,w)
	}
	// Check if P in edge region of BC, if so return projection of P onto BC
	float va = d3*d6 - d5*d4;
	if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) 
	{
		float w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
		barycentriqueCoords = glm::vec3(0, 1.0 - w, w);
		return b + w * (c - b); // barycentric coordinates (0,1-w,w)
	}
	// P inside face region. Compute Q through its barycentric coordinates (u,v,w)
	float denom = 1.0f / (va + vb + vc);
	float v = vb * denom;
	float w = vc * denom;
	float u = 1.0f - v - w;
	barycentriqueCoords = glm::vec3(u, v, w);
	return a + ab * v + ac * w; // = u*a + v*b + w*c, u = va * denom = 1.0f - v - w


}

DetailsAdder::DetailsAdder(std::string name, KernelList* kernels, Engine* engineToUse, Camera* cameraToApplyOn, LightingModelGL* lightList) :
MaterialGL(name,"DetailsAdder")
{
	
	
	// Pointer storage
	this->kernels = kernels;
	this->engineToUse = engineToUse;
	this->cameraToApplyOn = cameraToApplyOn;
	this->lightingModel = lightList;

	// Surface data transfer
	precomputedModel = new TextureGenerator(name + "-ModelComputation");
	perPixelComputing = new ShellRayCompute("PixelToUV mapping");

	meshDataFBO = new GPUFBO(name + "-meshDataFBO");
	meshDataFBO->create(1024, 1024, 1, false, GL_RGBA16F, GL_TEXTURE_2D_ARRAY, 4);
	pixelsData = new GPUFBO(name + "-pixelsDataFBO");
	pixelsData->create(engineToUse->getWidth(), engineToUse->getHeight(), 2, false, GL_RGBA16F, GL_TEXTURE_2D, 1); // front face et backface des UV

	// Vertex shader uniforms
	vp_m44_Object_toScreen = vp->uniforms()->getGPUmat4("m44_Object_toScreen");
	vp_f_Grid_height = vp->uniforms()->getGPUfloat("f_Grid_height");


	// Fragment shader uniforms
	fp_v2_Screen_size = fp->uniforms()->getGPUvec2("v2_Screen_size");
	fp_v3_Object_cameraPosition = fp->uniforms()->getGPUvec3("v3_Object_cameraPosition");
	
	fp_m44_Object_toCamera = fp->uniforms()->getGPUmat4("m44_Object_toCamera");
	fp_m44_Object_toScreen = fp->uniforms()->getGPUmat4("m44_Object_toScreen");
	fp_m44_Object_toWorld = fp->uniforms()->getGPUmat4("m44_Object_toWorld");
	
	fp_v3_AABB_min = fp->uniforms()->getGPUvec3("v3_AABB_min");
	fp_v3_AABB_max = fp->uniforms()->getGPUvec3("v3_AABB_max");
	fp_f_Grid_height = fp->uniforms()->getGPUfloat("f_Grid_height");
	
	fp_smp_DepthTexture		= fp->uniforms()->getGPUsampler("smp_DepthTexture");
	fp_smp_models			= fp->uniforms()->getGPUsampler("smp_models");
	fp_smp_densityMaps		= fp->uniforms()->getGPUsampler("smp_densityMaps");
	fp_smp_scaleMaps		= fp->uniforms()->getGPUsampler("smp_scaleMaps");
	fp_smp_distributionMaps = fp->uniforms()->getGPUsampler("smp_distributionMaps");
	fp_smp_surfaceData		= fp->uniforms()->getGPUsampler("smp_surfaceData");
	fp_smp_voxel_UV			= fp->uniforms()->getGPUsampler("smp_voxel_UV");
	fp_smp_voxel_distanceField = fp->uniforms()->getGPUsampler("smp_voxel_distanceField");

	fp_renderKernels = fp->uniforms()->getGPUbool("renderKernels");
	fp_iv3_VoxelGrid_subdiv = fp->uniforms()->getGPUivec3("iv3_VoxelGrid_subdiv");

	fp_activeAA = fp->uniforms()->getGPUbool("activeAA");
	fp_activeShadows = fp->uniforms()->getGPUbool("activeShadows");
	fp_testFactor = fp->uniforms()->getGPUfloat("testFactor");
	fp_dMinFactor = fp->uniforms()->getGPUfloat("dMinFactor");
	fp_dMaxFactor = fp->uniforms()->getGPUfloat("dMaxFactor");

	fp_renderGrid = fp->uniforms()->getGPUbool("renderGrid");

	fp_modeSlicing = fp->uniforms()->getGPUbool("modeSlicing");

	cameraToApplyOn = Scene::getInstance()->camera();
	this->engineToUse = engineToUse;

	// Binding points
	int bindingCounter = 0;
	fp_smp_DepthTexture->Set(bindingCounter);
	fp_smp_models->Set(++bindingCounter);
	fp_smp_densityMaps->Set(++bindingCounter);
	fp_smp_scaleMaps->Set(++bindingCounter);
	fp_smp_distributionMaps->Set(++bindingCounter);
	fp_smp_surfaceData->Set(++bindingCounter);
	fp_smp_voxel_UV->Set(++bindingCounter);
	fp_smp_voxel_distanceField->Set(++bindingCounter);

	this->kernels = kernels;
	lightingModel = lightList;

	precomputationDone = false;

	fp->uniforms()->mapBufferToBlock(lightingModel->getBuffer(), "LightingUBO");
	fp->uniforms()->mapBufferToBlock(this->kernels->getBuffer(), "KernelUBO");
	fp->uniforms()->mapBufferToStorageBlock(lightingModel->getBuffer(), "LightingBuffer");
	fp->uniforms()->mapBufferToStorageBlock(kernels->getBuffer(), "KernelBuffer");

	// Default value (settings dans l'engine)
	voxelSubdiv = glm::ivec3(32,32,32);

}

void DetailsAdder::setVoxelGridSize(glm::ivec3 gridAxesSize)
{
	this->voxelSubdiv = gridAxesSize;
}

void DetailsAdder::render(Node* o)
{
	if (m_ProgramPipeline)
	{
		if (precomputationDone == false)
		{
			precomputeData(o);
		}

		// pas la bonne solution
		//pixelsData->enable();
		//pixelsData->drawBuffer(0);
		//perPixelComputing->renderSurface(o);
		//pixelsData->drawBuffer(1);
		//perPixelComputing->renderSurfaceBack(o);
		//pixelsData->disable();

		fp_modeSlicing->Set(kernels->modeSlicing);

		glm::vec3 camPos = Scene::getInstance()->camera()->convertPtTo(glm::vec3(0.0, 0.0, 0.0), o->frame());
		// Vertex stage
		vp_m44_Object_toScreen->Set(o->frame()->getTransformMatrix());
		vp_f_Grid_height->Set(kernels->getGridHeight());
		

		//fragment stage
		fp_v2_Screen_size->Set(glm::vec2(engineToUse->getWidth(), engineToUse->getHeight()));
		fp_v3_Object_cameraPosition->Set(camPos);
		
		fp_m44_Object_toCamera->Set(Scene::getInstance()->camera()->getModelViewMatrix(o->frame()));
		fp_m44_Object_toScreen->Set(o->frame()->getTransformMatrix());
		fp_m44_Object_toWorld->Set(o->frame()->getRootMatrix());
		fp_v3_AABB_min->Set(o->getModel()->getGeometricModel()->getShellAABB()->getMin());
		fp_v3_AABB_max->Set(o->getModel()->getGeometricModel()->getShellAABB()->getMax());
		fp_testFactor->Set(kernels->testFactor);
		fp_dMinFactor->Set(kernels->dMinFactor);
		fp_dMaxFactor->Set(kernels->dMaxFactor);

		fp_f_Grid_height->Set(kernels->getGridHeight());

		fp_activeAA->Set(kernels->activeAA);
		fp_activeShadows->Set(kernels->activeShadows);
		fp_iv3_VoxelGrid_subdiv->Set(voxelSubdiv);
		fp_renderGrid->Set(kernels->renderGrid);
		fp_renderKernels->Set(kernels->renderKernels);


		colorFBO->bindColorTexture(fp_smp_DepthTexture->getValue(), 1);
		kernels->bindModels(fp_smp_models->getValue());
		kernels->bindDensityMaps(fp_smp_densityMaps->getValue());
		kernels->bindScaleMaps(fp_smp_scaleMaps->getValue());
		kernels->bindDistributionMaps(fp_smp_distributionMaps->getValue());
		meshDataFBO->bindColorTexture(fp_smp_surfaceData->getValue());

		//voxelUVTexture->bind(fp_smp_voxel_UV->getValue());
		//voxelDistanceField->bind(fp_smp_voxel_distanceField->getValue());

		// -- Draw call
		m_ProgramPipeline->bind();
		((ModelGL*)o->getModel())->bboxGL->drawGeometry();
		m_ProgramPipeline->release();
		// */

		// Releasing the data
		//voxelUVTexture->release();
		//voxelDistanceField->release();
		meshDataFBO->releaseColorTexture();
		kernels->releaseTextures();
		colorFBO->releaseColorTexture();

		//meshDataFBO->display(glm::vec4(0, 0, 0.1, 0.1), 0);
		//pixelsData->display(glm::vec4(0, 0, 1, 1), 0);
		
	}
}


void DetailsAdder::precomputeData(Node* target)
{
	
	meshDataFBO->enable();
	meshDataFBO->bindLayerToBuffer(0, 0);
	meshDataFBO->bindLayerToBuffer(1, 1);
	meshDataFBO->bindLayerToBuffer(2, 2);
	meshDataFBO->bindLayerToBuffer(3, 3);
	meshDataFBO->drawBuffers(4);
	precomputedModel->render(target);
	meshDataFBO->disable();
	computeOctree(target->getModel()->getGeometricModel()->getShellAABB(), target->getModel()->getGeometricModel());
	precomputationDone = true;
}

void DetailsAdder::setBackgroundFBO(GPUFBO* color)
{
	this->colorFBO = color;
}


void DetailsAdder::computeOctree(AxisAlignedBoundingBox* shellAABB, GeometricModel* ObjectModel)
{
	
	AxisAlignedBoundingBox* toUse = shellAABB;
	GeometricModel* temp = ObjectModel;

	/* Algo brute :
	Pour chaque voxel,
		Pour chaque sommet
			On calcul le point de la surface le plus proche de celui ci, et pour ce point on calcul une direction et des UVs

	Pour chaque voxel,
		On calcul les UV min et max a partir des sommets
	*/


	std::cout << "Init Voxel grid - ";
	int VoxelGridSize = voxelSubdiv.x * voxelSubdiv.y * voxelSubdiv.z;
	int latterSize = (voxelSubdiv.x + 1) * (voxelSubdiv.y + 1) * (voxelSubdiv.z + 1);
	// subdiv de la shell AABB
	glm::vec3 fullVector = toUse->getHalfVector() * 2.0f;
	glm::vec3 cellSizes = (fullVector) / glm::vec3(voxelSubdiv);

	// Idée : considérer plutot la grille de voxel comme une grille de points (représentant les sommets des voxels)
	// On peut ensuite reconstruire la grille de voxel en speed à partir de la grille de point

	voxelGrid = new std::vector<CPUVoxel*>(VoxelGridSize);

	// modification
	latter = new std::vector<gridPoint*>(latterSize);

	for (int z_i = 0; z_i <= voxelSubdiv.z; z_i++)
	for (int y_i = 0; y_i <= voxelSubdiv.y; y_i++)
	for (int x_i = 0; x_i <= voxelSubdiv.x; x_i++)
	{
		int iL = x_i + (voxelSubdiv.x + 1) * y_i + (voxelSubdiv.x + 1) * (voxelSubdiv.y + 1) * z_i;

		latter->at(iL) = new gridPoint();
		latter->at(iL)->position = toUse->getMin() + glm::vec3(x_i, y_i, z_i) * cellSizes;
		latter->at(iL)->UV = glm::vec2(0, 0);
		latter->at(iL)->surfacePoint = toUse->getMax() + fullVector; // point hors cadre d'évaluation
		latter->at(iL)->isInShell = false; // boolean pour tester si la cellule à déjà été évalué

	}

	for (int z_i = 0; z_i < voxelSubdiv.z; z_i++)
	for (int y_i = 0; y_i < voxelSubdiv.y; y_i++)
	for (int x_i = 0; x_i < voxelSubdiv.x; x_i++)
	{
		int iV = x_i + (voxelSubdiv.x) * y_i + (voxelSubdiv.x) * (voxelSubdiv.y) * z_i;
		voxelGrid->at(iV) = new CPUVoxel();
		//voxelGrid->push_back(new Voxel());
		// default values
		voxelGrid->at(iV)->dir_surface = glm::vec3(0);
		voxelGrid->at(iV)->min_UV = glm::vec2(2);
		voxelGrid->at(iV)->max_UV = glm::vec2(-1);
		voxelGrid->at(iV)->closest_UV = glm::vec2(-1);
		voxelGrid->at(iV)->min_distance = glm::length(fullVector) * 2.0f;

		// lien entre un voxel et les 8 points de la grille
		glm::ivec3 index = glm::ivec3(0);
		int iL = 0;

		index = glm::ivec3(x_i, y_i, z_i); // 1er sommet : min
		iL = index.x + (voxelSubdiv.x + 1) * index.y + (voxelSubdiv.x + 1) * (voxelSubdiv.y + 1) * index.z;
		voxelGrid->at(iV)->sommets[0] = latter->at(iL);

		index = glm::ivec3(x_i + 1, y_i, z_i); // 2e sommet : min + x
		iL = index.x + (voxelSubdiv.x + 1) * index.y + (voxelSubdiv.x + 1) * (voxelSubdiv.y + 1) * index.z;
		voxelGrid->at(iV)->sommets[1] = (latter->at(iL));

		index = glm::ivec3(x_i + 1, y_i + 1, z_i); // 3e sommet : min + xy
		iL = index.x + (voxelSubdiv.x + 1) * index.y + (voxelSubdiv.x + 1) * (voxelSubdiv.y + 1) * index.z;
		voxelGrid->at(iV)->sommets[2] = (latter->at(iL));

		index = glm::ivec3(x_i, y_i + 1, z_i); // 4e sommet : min + y
		iL = index.x + (voxelSubdiv.x + 1) * index.y + (voxelSubdiv.x + 1) * (voxelSubdiv.y + 1) * index.z;
		voxelGrid->at(iV)->sommets[3] = (latter->at(iL));

		index = glm::ivec3(x_i, y_i, z_i + 1); // 5e sommet : min + z
		iL = index.x + (voxelSubdiv.x + 1) * index.y + (voxelSubdiv.x + 1) * (voxelSubdiv.y + 1) * index.z;
		voxelGrid->at(iV)->sommets[4] = (latter->at(iL));

		index = glm::ivec3(x_i + 1, y_i, z_i + 1); // 6e sommet : min + xz
		iL = index.x + (voxelSubdiv.x + 1) * index.y + (voxelSubdiv.x + 1) * (voxelSubdiv.y + 1) * index.z;
		voxelGrid->at(iV)->sommets[5] = (latter->at(iL));

		index = glm::ivec3(x_i + 1, y_i + 1, z_i + 1); // 7e sommet : min + xyz
		iL = index.x + (voxelSubdiv.x + 1) * index.y + (voxelSubdiv.x + 1) * (voxelSubdiv.y + 1) * index.z;
		voxelGrid->at(iV)->sommets[6] = (latter->at(iL));

		index = glm::ivec3(x_i, y_i + 1, z_i + 1); // 8e sommet : min + yz
		iL = index.x + (voxelSubdiv.x + 1) * index.y + (voxelSubdiv.x + 1) * (voxelSubdiv.y + 1) * index.z;
		voxelGrid->at(iV)->sommets[7] = (latter->at(iL));


	}

	std::cout << " done " << endl;

	// brute force
	//std::cout << "Fill voxel grid : " << temp->listFaces.size() << " Faces to treat -";
	//for (int i = 0; i < temp->listFaces.size(); i++)
	//{
	//	//cout << " face " << i << " / " << temp->listFaces.size() << endl;
	//	glm::vec3 s1 = temp->listVertex[temp->listFaces[i].s1];
	//	glm::vec3 s2 = temp->listVertex[temp->listFaces[i].s2];
	//	glm::vec3 s3 = temp->listVertex[temp->listFaces[i].s3];
	//
	//	glm::vec2 u1 = glm::swizzle<glm::X,glm::Y>(temp->listCoords[temp->listFaces[i].s1]);
	//	glm::vec2 u2 = glm::swizzle<glm::X, glm::Y>(temp->listCoords[temp->listFaces[i].s2]);
	//	glm::vec2 u3 = glm::swizzle<glm::X, glm::Y>(temp->listCoords[temp->listFaces[i].s3]);
	//	
	//
	//	// Pour chaque point contenu 
	//	for (int j = 0; j < latter->size(); j++)
	//	{
	//		
	//		glm::vec3 baryCoords = glm::vec3(0, 0, 0);
	//		glm::vec3 closestFacePoint = triangleClosestPoint(latter->at(j)->position, s1, s2, s3, baryCoords);
	//		if (glm::distance(closestFacePoint, latter->at(j)->position) < glm::distance(latter->at(j)->surfacePoint, latter->at(j)->position))
	//		{
	//			latter->at(j)->surfacePoint = closestFacePoint;
	//			latter->at(j)->UV = u1 * baryCoords.x + u2 * baryCoords.y + u3 * baryCoords.z;
	//			latter->at(j)->isInShell = true;
	//		}
	//	}
	//}


	for (int i = 0; i < temp->listFaces.size(); i++)
	{
		//cout << " face " << i << " / " << temp->listFaces.size() << endl;
		glm::vec3 s1 = temp->listVertex[temp->listFaces[i].s1];
		glm::vec3 s2 = temp->listVertex[temp->listFaces[i].s2];
		glm::vec3 s3 = temp->listVertex[temp->listFaces[i].s3];
	

		glm::vec2 u1; // = glm::swizzle<glm::X, glm::Y>(temp->listCoords[temp->listFaces[i].s1]);
		u1.x = temp->listCoords[temp->listFaces[i].s1].x;
		u1.y = temp->listCoords[temp->listFaces[i].s1].y;
		glm::vec2 u2 = glm::vec2(temp->listCoords[temp->listFaces[i].s2].x, temp->listCoords[temp->listFaces[i].s2].y);
		glm::vec2 u3 = glm::vec2(temp->listCoords[temp->listFaces[i].s3].x, temp->listCoords[temp->listFaces[i].s3].y);
	
	
		// Rajout : calcul du prisme et de sa boite englobante
		glm::vec3 se1 = temp->listVertex_extruder[temp->listFaces[i].s1];
		glm::vec3 se2 = temp->listVertex_extruder[temp->listFaces[i].s2];
		glm::vec3 se3 = temp->listVertex_extruder[temp->listFaces[i].s3];
	
		glm::vec3 prismMin = s1;
		glm::vec3 prismMax = s1;
	
		// calcul d'une boite englobante pour le prisme (espace objet) (min et max de chaque composante de chaque point)
		for (int c = 0; c < 3; c++)
		{
			// MIN
			if (s1[c] < prismMin[c]) prismMin[c] = s1[c];
			if (se1[c] < prismMin[c]) prismMin[c] = se1[c];
	
			if (s2[c] < prismMin[c]) prismMin[c] = s2[c];
			if (se2[c] < prismMin[c]) prismMin[c] = se2[c];
	
			if (s3[c] < prismMin[c]) prismMin[c] = s3[c];
			if (se3[c] < prismMin[c]) prismMin[c] = se3[c];
	
			// MAX
			if (s1[c] > prismMax[c]) prismMax[c] = s1[c];
			if (se1[c] > prismMax[c]) prismMax[c] = se1[c];
	
			if (s2[c] > prismMax[c]) prismMax[c] = s2[c];
			if (se2[c] > prismMax[c]) prismMax[c] = se2[c];
	
			if (s3[c] > prismMax[c]) prismMax[c] = s3[c];
			if (se3[c] > prismMax[c]) prismMax[c] = se3[c];
		}
	
		// min et max dans l'espace des voxels
		glm::vec3 minPoint = (((prismMin - toUse->getMin()) / fullVector)) * glm::vec3(voxelSubdiv);
		glm::vec3 maxPoint = (((prismMax - toUse->getMin()) / fullVector)) * glm::vec3(voxelSubdiv);
	
		// calcul des indices dans la grille de point
		minPoint = clamp(glm::floor(minPoint) - glm::vec3(1.0), glm::vec3(0), glm::vec3(voxelSubdiv));
		maxPoint = clamp(glm::floor(maxPoint + glm::vec3(1.0)), glm::vec3(0), glm::vec3(voxelSubdiv));
	
		// Pour chaque point contenu 
	
		for (int z_i = minPoint.z; z_i <= maxPoint.z; z_i++)
		for (int y_i = minPoint.y; y_i <= maxPoint.y; y_i++)
		for (int x_i = minPoint.x; x_i <= maxPoint.x; x_i++)
		{
			int iL = x_i + (voxelSubdiv.x + 1) * y_i + (voxelSubdiv.x + 1) * (voxelSubdiv.y + 1) * z_i;
			glm::vec3 baryCoords = glm::vec3(0, 0, 0);
			glm::vec3 closestFacePoint = triangleClosestPoint(latter->at(iL)->position, s1, s2, s3, baryCoords);
			if (glm::distance(closestFacePoint, latter->at(iL)->position) < glm::distance(latter->at(iL)->surfacePoint, latter->at(iL)->position))
			{
				latter->at(iL)->surfacePoint = closestFacePoint;
				latter->at(iL)->UV = u1 * baryCoords.x + u2 * baryCoords.y + u3 * baryCoords.z;
				latter->at(iL)->isInShell = true;
			}
		}
	}


	for (int i = 0; i < voxelGrid->size(); i++)
	{

		for (int j = 0; j < 8; j++)
		{
			if (voxelGrid->at(i)->sommets[j]->isInShell)
			{
				
				float surfaceDistance = glm::distance(voxelGrid->at(i)->sommets[j]->position, voxelGrid->at(i)->sommets[j]->surfacePoint);
				if (surfaceDistance < voxelGrid->at(i)->min_distance)
				{
					voxelGrid->at(i)->min_distance = surfaceDistance;
					voxelGrid->at(i)->closest_UV = voxelGrid->at(i)->sommets[j]->UV;
					if (surfaceDistance > 0.0)	
						voxelGrid->at(i)->dir_surface = glm::normalize(voxelGrid->at(i)->sommets[j]->position - voxelGrid->at(i)->sommets[j]->surfacePoint);
					else voxelGrid->at(i)->dir_surface = voxelGrid->at(i)->sommets[j]->position - voxelGrid->at(i)->sommets[j]->surfacePoint;
					
				}
				voxelGrid->at(i)->min_UV = glm::min(voxelGrid->at(i)->min_UV, voxelGrid->at(i)->sommets[j]->UV);
				voxelGrid->at(i)->max_UV = glm::max(voxelGrid->at(i)->max_UV, voxelGrid->at(i)->sommets[j]->UV);
			}
			
		}

	}

	// pour optimiser le résultat "de base" on réestime la grid height en fonction de la taille d'un voxel (taille de 2 voxels)
	// sauf que bof bof pour les résultats
	//kernels->setGridHeight(glm::length(cellSizes) * 0.5);

	std::cout << "GPU voxel buffer - ";
	voxelBuffer = new GPUBuffer("VoxelBuffer");
	voxelBuffer->create(voxelGrid->size() * 3 * sizeof(glm::vec4), GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, voxelBuffer->getBuffer());
	GPUVoxel* GPUVoxelGrid = (GPUVoxel*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, voxelGrid->size() * 3 * sizeof(glm::vec4), GL_MAP_WRITE_BIT);
	if (GPUVoxelGrid != NULL)
	{
		for (int i = 0; i < voxelGrid->size(); i++)
		{
			GPUVoxelGrid[i].data[0].x = voxelGrid->at(i)->min_UV.x;
			GPUVoxelGrid[i].data[0].y = voxelGrid->at(i)->min_UV.y;
			GPUVoxelGrid[i].data[0].z = voxelGrid->at(i)->max_UV.x;
			GPUVoxelGrid[i].data[0].w = voxelGrid->at(i)->max_UV.y;
			GPUVoxelGrid[i].data[1].x = voxelGrid->at(i)->min_distance;
			GPUVoxelGrid[i].data[1].y = voxelGrid->at(i)->dir_surface.x;
			GPUVoxelGrid[i].data[1].z = voxelGrid->at(i)->dir_surface.y;
			GPUVoxelGrid[i].data[1].w = voxelGrid->at(i)->dir_surface.z;
			GPUVoxelGrid[i].data[2].x = voxelGrid->at(i)->closest_UV.x;
			GPUVoxelGrid[i].data[2].y = voxelGrid->at(i)->closest_UV.y;
			GPUVoxelGrid[i].data[2].z = 0;
			GPUVoxelGrid[i].data[2].w = 0;

		}
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	}
	//glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, fullSize * sizeof(glm::vec4), gridV4);
	//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	std::cout << "Done" << endl;
	fp->uniforms()->mapBufferToStorageBlock(voxelBuffer, "VoxelBuffer");


}
