
#include "Voxel.h"
#include <algorithm>
#include "myMathUtils.hpp"
#define NBDONNEESMAX 100
Voxel::Voxel()
{
	data.position = glm::vec4(0.0);
	data.normale = glm::vec4(0.0);
	data.tangente = glm::vec4(0.0);
	data.coord_texture = glm::vec4(0.0);
	data.coord_surface = glm::vec4(0.0);
}

Voxel::Voxel(VoxelData & toCopy)
{
	data = toCopy;
}

Voxel::Voxel(
	glm::vec4 & position, 
	glm::vec4 & normale, 
	glm::vec4 & tangente, 
	glm::vec4 & coord_texture, 
	glm::vec4 & coord_surface)
{
	data.position = position;
	data.normale = normale;
	data.tangente = tangente;
	data.coord_texture = coord_texture;
	data.coord_surface = coord_surface;
}

Voxel::~Voxel(){ } // Automatic destructor


VoxelData* Voxel::getAdressOfData()
{
	return &data;
}


VoxelData Voxel::getCopyOfData()
{
	return data;
}

void Voxel::changePosition(
	glm::vec4 & position)
{
	data.position = position;
}

void Voxel::changeNormale(
	glm::vec4 & normale)
{
	data.normale = normale;
}

void Voxel::changeTangente(
	glm::vec4 & tangente)
{
	data.tangente = tangente;
}

void Voxel::changeCoord_texture(
	glm::vec4 & coord_texture)
{
	data.coord_texture = coord_texture;
}


void Voxel::changeCoord_surface(
	glm::vec4 & coord_surface)
{
	data.coord_surface = coord_surface;
}


VoxelVector::VoxelVector()
{
	computeDone = false;
}
VoxelVector::VoxelVector(
	GeometricModel* fromGeometry,
	int subdivision,
	float shellHeight)
{
	subdivision = 4;
	int taille = pow(8.0, subdivision);
	cout << "subdivision : " << subdivision << endl;
	cout << "nombre de voxels : " << taille << endl;
	/*for (int i = 0; i < taille; i++)
	{
		this->push_back(Voxel());
	}*/
	computeDone = false;
	this->compute(fromGeometry,subdivision,shellHeight);
}

VoxelVector::VoxelVector(int subdivision)
{
	int taille = pow(8.0, subdivision);
	cout << "subdivision : " << subdivision << endl;
	cout << "nombre de voxels : " << taille << endl;
	for(int i = 0; i < taille; i++)
	{
		this->push_back(Voxel());
	}
	//this->compute(fromGeometry, subdivision, shellHeight)
}

void VoxelVector::compute(
	GeometricModel* fromGeometry,
	int subdivision,
	float shellHeight)
{
	if (fromGeometry != NULL)
	{
		// Extrusion + Bounding Box
		minCoord = fromGeometry->listVertex[0];
		maxCoord = minCoord;

		std::cout << "Extrusion Bounding box - ";
		for (int i = 0; i < fromGeometry->listVertex.size(); i++)
		{
			listExtrudedVertex.push_back(fromGeometry->listVertex[i] + shellHeight * fromGeometry->listNormals[i]);

			for (int j = 0; j < 3; j++)
			{
				minCoord[j] = minCoord[j] > listExtrudedVertex[i][j] ? listExtrudedVertex[i][j] : minCoord[j];
				minCoord[j] = minCoord[j] > fromGeometry->listVertex[i][j] ? fromGeometry->listVertex[i][j] : minCoord[j];

				maxCoord[j] = maxCoord[j] < listExtrudedVertex[i][j] ? listExtrudedVertex[i][j] : maxCoord[j];
				maxCoord[j] = maxCoord[j] < fromGeometry->listVertex[i][j] ? fromGeometry->listVertex[i][j] : maxCoord[j];
			}
		}

		cout << minCoord.x << " " << minCoord.y << " " << minCoord.z << endl;
		cout << maxCoord.x << " " << maxCoord.y << " " << maxCoord.z << endl;
		std::cout << "done" << std::endl;;
		// Transformation de la bounding box en bounding cube :
		glm::vec3 center = (minCoord + maxCoord) * glm::vec3(0.5);
		glm::vec3 halfVector = (maxCoord - minCoord ) * glm::vec3(0.5);
		float halfSize = max(max(halfVector.x, halfVector.y), halfVector.z);
		halfVector = glm::vec3(halfSize);
		minCoord = center - halfVector;
		maxCoord = center + halfVector;

		cout << minCoord.x << " " << minCoord.y << " " << minCoord.z << endl;
		cout << maxCoord.x << " " << maxCoord.y << " " << maxCoord.z << endl;
		
		int nbVoxelPerAxis = pow(2.0, subdivision);

		cout << nbVoxelPerAxis * nbVoxelPerAxis * nbVoxelPerAxis << endl;

		voxelHalfSize = halfSize / (float)nbVoxelPerAxis;

	//	AABB = new GeometricBoundingBox(NULL);
	//	AABB->compute();

		//Voxels grille compute shader


		/////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////BUFFER GEOMETRY///////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////////////////////
		cout << "SSBO liste geometry" << endl;

		// Création du buffer de voxel
		GPUBuffer* geometryBuffer = new GPUBuffer("geometryBuffer");
		//Taille du buffer
		int geometryBufferSize = sizeof(geometry)* fromGeometry->listCoords.size();
		//Création de la SSBO
		geometryBuffer->create(geometryBufferSize, GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW);
		geometryBuffer->setBindingPoint(5);
		geometryBuffer->bind();
		//pointeur sur la structure geometryArray dans le GPU
		geometry *ptr2 = (geometry*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(geometry)* fromGeometry->listCoords.size(), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		for (int i = 0; i < fromGeometry->listCoords.size(); i++)
		{
			ptr2[i].point = glm::vec4(fromGeometry->listVertex[i].x, fromGeometry->listVertex[i].y, fromGeometry->listVertex[i].z, 0);
			ptr2[i].normale = glm::vec4(fromGeometry->listNormals[i].x, fromGeometry->listNormals[i].y, fromGeometry->listNormals[i].z, 0.0);
			ptr2[i].tangente = glm::vec4(fromGeometry->listTangents[i].x, fromGeometry->listTangents[i].y, fromGeometry->listTangents[i].z, 0.0);
			ptr2[i].coord = glm::vec4(fromGeometry->listCoords[i].x, fromGeometry->listCoords[i].y, fromGeometry->listCoords[i].z, 0.0);
			ptr2[i].pointextr = glm::vec4(listExtrudedVertex[i].x, listExtrudedVertex[i].y, listExtrudedVertex[i].z, 0);
		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		cout << "fin SSBO liste points" << endl;


		///////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////BUFFER FACES GEOMETRY/////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////
		cout << "SSBO liste faces " << endl;

		// Création du buffer de voxel
		GPUBuffer* facesBuffer = new GPUBuffer("FacesBuffer");
		//Taille du buffer
		int facesBufferSize = sizeof(Triangle)* fromGeometry->listFaces.size() + sizeof(glm::ivec4) + sizeof(Cube);
		//Création de la SSBO
		facesBuffer->create(facesBufferSize, GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW);
		facesBuffer->setBindingPoint(6);
		facesBuffer->bind();
		//pointeur sur la structure geometryArray dans le GPU
		geometryArray* ptr3 = (geometryArray*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
		ptr3->size= glm::vec4(fromGeometry->listFaces.size(),0,0,0);
		//cout << "taille : " << ptr3->size.x << endl;
		ptr3->cube.maxCoord = glm::vec4(maxCoord.x, maxCoord.y, maxCoord.z, 0);
		ptr3->cube.minCoord = glm::vec4(minCoord.x, minCoord.y, minCoord.z, 0);
		//printf("Halfvector : %f, %f, %f", halfVector.x, halfVector.y, halfVector.z);
		ptr3->cube.halfVector = glm::vec4(halfVector.x, halfVector.y, halfVector.z, 0);
		ptr3->cube.nbVoxelPerAxis = glm::ivec4(nbVoxelPerAxis, 0, 0, 0);

		Triangle *triangles = (Triangle*)malloc(sizeof(Triangle) * fromGeometry->listFaces.size());
		for (int i = 0; i < fromGeometry->listFaces.size(); i++)
		{
			triangles[i].s.x = fromGeometry->listFaces[i].s1;
			triangles[i].s.y = fromGeometry->listFaces[i].s2;
			triangles[i].s.z = fromGeometry->listFaces[i].s3;
			triangles[i].s.w = 0;
		}

		memcpy(ptr3->triangles, &(triangles[0]), sizeof(Triangle)* fromGeometry->listFaces.size());

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		cout << "fin SSBO liste faces" << endl;

		


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////COMPUTE SHADER TAILLE VOXEL PAR FACE/////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		cout << "compute shader create gridvoxels..." << endl;
		//compute shader pour la voxelisation
		GLProgramPipeline* computePipeline = new GLProgramPipeline("CreateVoxels");
		GLProgram* computeProgram = new GLProgram("-OctreeVoxels", GL_COMPUTE_SHADER);
		computePipeline->useProgramStage(GL_COMPUTE_SHADER_BIT, computeProgram);
		computePipeline->link();
		computePipeline->bind();
		glDispatchCompute((int)(fromGeometry->nb_faces+127)/128, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		computePipeline->release();

		cout << "Fin compute shader" << endl;



		///////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////CALCUL NOMBRE VOXELS PAR FACE/////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////////////////////////////
		cout << "Calcul nb voxels par faces" << endl;
		//taille du buffer de voxels
		int voxelBufferSize = sizeof(glm::ivec4);

		facesBuffer->bind();
		geometryArray* gA = (geometryArray*)malloc(sizeof(Triangle)* fromGeometry->listFaces.size() + sizeof(glm::ivec4) + sizeof(Cube));
		gA = (geometryArray*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
		vector<int> nbVoxelsPerFaces(fromGeometry->listFaces.size());
		int n;
		int nTotal = 0;

		if (gA != NULL)
		{
			for (int i = 0; i < fromGeometry->listFaces.size(); i++)
			{
				gA->triangles[i].offsetVoxel = glm::ivec4(nTotal, 0, 0, 0);
				n = (gA->triangles[i].maxPoint.x - gA->triangles[i].minPoint.x + 1) * (gA->triangles[i].maxPoint.y - gA->triangles[i].minPoint.y + 1) * (gA->triangles[i].maxPoint.z - gA->triangles[i].minPoint.z + 1);
				nTotal += n;
				voxelBufferSize += sizeof(Voxel) * n;
				nbVoxelsPerFaces[i] = n;
			}
		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////BUFFER VOXELS//////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		cout << "debut SSBO liste voxels" << endl;
		// Création du buffer de voxel
		GPUBuffer* voxelBuffer = new GPUBuffer("VoxelBuffer");

		//on créé le buffer de voxels et on le bind sur le canal 4
		voxelBuffer->create(voxelBufferSize, GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW);
		voxelBuffer->setBindingPoint(4);
		voxelBuffer->bind();

		// Ptr to VoxelArray structure in GPU
		VoxelArray* ptr = (VoxelArray*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);

		ptr->size.x = nTotal;
		ptr->size.y = 0;
		ptr->size.z = 0;
		ptr->size.w = 0;
		Voxel *voxs = (Voxel*)malloc(sizeof(Voxel) * nTotal);
		memcpy(ptr->bufferContent, voxs, sizeof(Voxel)* nTotal);

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
		cout << "fin SSBO liste voxels" << endl;


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////COMPUTE SHADER CREATE GRID VOXEL/////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		cout << "compute shader create grid voxels..." << endl;
		//compute shader pour la voxelisation
		computePipeline = new GLProgramPipeline("CreateVoxels");
		computeProgram = new GLProgram("-OctreeVoxels2", GL_COMPUTE_SHADER);
		computePipeline->useProgramStage(GL_COMPUTE_SHADER_BIT, computeProgram);
		computePipeline->link();
		computePipeline->bind();
		glDispatchCompute((int)(fromGeometry->nb_faces + 127) / 128, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		computePipeline->release();

		cout << "Fin compute shader" << endl;


		/*voxelBuffer->bind();
		VoxelArray* voxelArray = (VoxelArray*)malloc(sizeof(Voxel) * nTotal + sizeof(glm::ivec4));
		voxelArray = (VoxelArray*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		cout << "affichage GPU coord surface voxel" << endl;
		for (int i = nTotal-1; i > nTotal-40; i--)
		{
			glm::vec4 cs = voxelArray->bufferContent[i].getVoxelData().coord_surface;
			cout << cs.x << "   " << cs.y << "  "  << cs.z << "  " << cs.w << endl;
		}*/

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////BUFFER int toDelete//////////////////////////////////////////////////////////////
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////
		cout << "debut SSBO int toDelete" << endl;

		// Création du buffer de voxel
		GPUBuffer* deleteBuffer = new GPUBuffer("DeleteBuffer");
		//Taille du buffer
		int deleteBufferSize = sizeof(deleteArray)* nTotal;
		//Création de la SSBO
		deleteBuffer->create(deleteBufferSize, GL_SHADER_STORAGE_BUFFER, GL_DYNAMIC_DRAW);
		deleteBuffer->setBindingPoint(15);
		deleteBuffer->bind();
		//pointeur sur la structure geometryArray dans le GPU
		deleteArray *ptrtmp = (deleteArray*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(deleteArray)* nTotal, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
		for (int i = 0; i < nTotal; i++)
		{
			ptrtmp[i].del = glm::ivec4(0);
		}

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		cout << "fin SSBO liste deletes" << endl;


		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////COMPUTE SHADER SUPPRESSION DOUBLONS VOXELS/////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		int step = 0;
		if (nTotal > NBDONNEESMAX)
			step = glm::ceil((float)nTotal / (float)NBDONNEESMAX);
		cout << step << endl;
		GPUint *offsetDonnees;
		cout << "compute shader Voxelization..." << endl;
		//compute shader pour la voxelisation
		computePipeline = new GLProgramPipeline("Voxelization");
		computeProgram = new GLProgram("-Voxelization", GL_COMPUTE_SHADER);
		offsetDonnees = computeProgram->uniforms()->getGPUint("offsetDonnee");
		computePipeline->useProgramStage(GL_COMPUTE_SHADER_BIT, computeProgram);
		computePipeline->link();
		//on le fait en plusieurs étapes ca trop de données à traiter
		for (int i = 1; i <= step; i++)
		{
			computePipeline->bind();
			//cout << "i : " << i << endl;
			offsetDonnees->Set(i);
			glDispatchCompute((nTotal + 511) / 512, 1, 1);
			//glDispatchComputeGroupSizeARB((nTotal + 511) / 512, 1, 1, 512, 0, 0);
			glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
			computePipeline->release();
		}
		cout << "Fin compute shader" << endl;

		////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		///////////////////////////////////SUPPRESSION DOUBLONS VOXELS/////////////////////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		

		
		voxelBuffer->bind();
		VoxelArray* voxelArray = (VoxelArray*)malloc(sizeof(Voxel) * nTotal + sizeof(glm::ivec4));
		voxelArray = (VoxelArray*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		deleteBuffer->bind();
		deleteArray* gdel = (deleteArray*)malloc(deleteBufferSize);
		gdel = (deleteArray*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		cout << "affichage delete GPU" << endl;

		for (int i = 0; i < nTotal; i++)
		{
			if (gdel[i].del.x == 0)
			{
				this->push_back(voxelArray->bufferContent[i]);
			}
		}
		
		cout << "total voxels : " << nTotal << " total voxels réduit : " << this->size() << endl;

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);


		cout << GL_MAX_COMPUTE_WORK_GROUP_COUNT << endl;
		cout << GL_MAX_COMPUTE_WORK_GROUP_SIZE << endl;
		cout << GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS << endl;
		
		computeDone = true;

		cout << "destruction SSBO ..." << endl;
		voxelBuffer->destroy();
		deleteBuffer->destroy();
		geometryBuffer->destroy();

		cout << "affichage Coordonnees bounding box : " << endl;
		cout << "position bounding box : (" << minCoord.x << "," << minCoord.y << "," << minCoord.z << ") ; (" << maxCoord.x << "," << maxCoord.y << "," << maxCoord.z << ")" << endl;
		/*for (int i = 0; i < 30; i++)
		{
			cout << "(" << voxelArray->bufferContent[i].getVoxelData().coord_texture.x << ", " << voxelArray->bufferContent[i].getVoxelData().coord_texture.y << ", " << voxelArray->bufferContent[i].getVoxelData().coord_texture.z << ")" << endl;
		}*/
	}
}

GeometricBoundingBox* VoxelVector::getGeometricBoundingBox()
{
	return AABB;
}

GPUVoxelVector::GPUVoxelVector(
	GeometricModel* fromGeometry,
	int SSBO_binding,
	int subdivision,
	float shellHeight)
	: VoxelVector(fromGeometry, subdivision, shellHeight)
{
	//cout << "Construction du GPUVector" << endl;
	//computeDone = false;
	/*if (false)
	try{
		// Création du buffer de voxel
		/*voxelBuffer = new GPUBuffer("VoxelBuffer");
		//taille du buffer de voxels
		int voxelBufferSize = sizeof(glm::ivec4) + sizeof(Voxel)* this->size();

		//on créé le buffer de voxels et on le bind sur le canal 4
		voxelBuffer->create(voxelBufferSize, GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW);
		voxelBuffer->setBindingPoint(4);
		voxelBuffer->bind();

		// Ptr to VoxelArray structure in GPU
		VoxelArray* ptr = (VoxelArray*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);

		ptr->size.x = this->size();
		ptr->size.y = 0;
		ptr->size.z = 0;
		ptr->size.w = 0;
		memcpy(ptr->bufferContent, &(this->at(0)), sizeof(Voxel)* this->size());

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

		cout << "compute shader..." << endl;
		//compute shader pour la voxelisation
		voxelBuffer->bind();
		computePipeline = new GLProgramPipeline("CreateVoxels");
		computeProgram = new GLProgram("-OctreeVoxels", GL_COMPUTE_SHADER);
		computePipeline->useProgramStage(GL_COMPUTE_SHADER_BIT, computeProgram);
		computePipeline->link();
		computePipeline->bind();
		glDispatchCompute((this->size() / 32) + 1, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		computePipeline->release();

		cout << "Fin compute shader" << endl;

		voxelBuffer->bind();
		Voxel* voxelArray = (Voxel*)malloc(this->size());
		voxelArray = (Voxel*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
		cout << "affichage" << endl;
		for (int i = 0; i < this->size(); i++)
		{
			cout << voxelArray[i].getVoxelData().normale.x << endl;
		}*/


		/*flagBuffer = new GPUBuffer("flagBuffer");
		int flagBufferSize = sizeof(int) * this->size();
		flagBuffer->create(flagBufferSize, GL_SHADER_STORAGE_BUFFER, GL_STREAM_READ);
		flagBuffer->setBindingPoint(5);

		// Bind SSBOs
		voxelBuffer->bind();
		flagBuffer->bind();
		// Compute shader for the reduction
		computePipeline = new GLProgramPipeline("Voxelizateur");
		computeProgram = new GLProgram("-Voxelization", GL_COMPUTE_SHADER);
		computePipeline->useProgramStage(GL_COMPUTE_SHADER_BIT, computeProgram);
		computePipeline->link();
		
		computePipeline->bind();
		glDispatchCompute((this->size() / 32) + 1, 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		computePipeline->release();

		flagBuffer->bind();
		int* flagArray = (int*)malloc(flagBufferSize);
		flagArray = (int*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);

		std::vector<Voxel> tempCopy;
		for (int i = 0; i < this->size(); i++)
		{
			if (flagArray[i] == 0) tempCopy.push_back(this->at(i));
		}
		//std::cout << "done" << std::endl;;
		this->swap(tempCopy);


	}
	catch (const std::exception & e)
	{
		std::cout << e.what() << std::endl;
	}
	


	if (this->size() > 0 & computeDone)
	{
		GPU_AABB = new BoundingBoxModelGL(AABB);
		loadDataToGPU(SSBO_binding);
	}*/
}

void GPUVoxelVector::loadDataToGPU(int SSBO_binding)
{
	if (computeDone)
	{
		voxelBuffer = new GPUBuffer("VoxelBuffer");
		int voxelBufferSize = sizeof(glm::ivec4) + sizeof(Voxel)* this->size();
	
		voxelBuffer->create(voxelBufferSize, GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW);
		voxelBuffer->setBindingPoint(SSBO_binding);

		voxelBuffer->bind();
	
		// Ptr to VoxelArray structure in GPU
		VoxelArray* ptr = (VoxelArray*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);

		ptr->size.x = this->size();
		ptr->size.y = 0;
		ptr->size.z = 0;
		ptr->size.w = 0;
		memcpy(ptr->bufferContent, &(this->at(0)), sizeof(Voxel)* this->size());

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	
		voxelBuffer->bind();
	}
}


GPUBuffer* GPUVoxelVector::getAdressOfBuffer()
{
	return voxelBuffer;
}



void GPUVoxelVector::changeVoxelBuffer(
	GPUBuffer* vBuffer)
{
	voxelBuffer = vBuffer;
}

BoundingBoxModelGL* GPUVoxelVector::getBoundingBoxModel()
{
	return GPU_AABB;
}


/*geometryBuffer->bind();
geometry* gB = (geometry*)malloc(geometryBufferSize);
gB = (geometry*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
cout << "affichage geometry GPU" << endl;
for (int i = 0; i < 10; i++)
{
cout << gB[i].point.x << "   " << gB[i].point.y << "   " << gB[i].point.z << endl;
}*/


/*facesBuffer->bind();
geometryArray* gA = (geometryArray*)malloc(sizeof(Triangle)* fromGeometry->listFaces.size() + sizeof(glm::ivec4) + sizeof(Cube));
gA = (geometryArray*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
if (gA == NULL)
cout << "gA -> NULL" << endl;
cout << "affichage geometryArray GPU" << endl;
for (int i = 0; i < 10; i++)
{
//cout << gA->size.x << "   " << gA->triangles[i].s.x << "   " << gA->triangles[i].s.y << "   " << gA->triangles[i].s.z << endl;
cout << gA->triangles[i].minPoint.x << "   " << gA->triangles[i].minPoint.y << "   " << gA->triangles[i].minPoint.z << endl;
cout << gA->triangles[i].maxPoint.x << "   " << gA->triangles[i].maxPoint.y << "   " << gA->triangles[i].maxPoint.z << endl;
}
cout << "GPU : nbVoxel axe " << gA->cube.nbVoxelPerAxis.x << "  " << gA->cube.halfVector.x << "   " << gA->cube.halfVector.y << "  " << gA->cube.halfVector.z << endl;
cout << "CPU : nbVoxel axe" << nbVoxelPerAxis << halfVector.x << "  " << halfVector.y << "  " << halfVector.z << endl;
cout << "GPU : maxCoord " << gA->cube.maxCoord.x << "    " << gA->cube.maxCoord.y << "   " << gA->cube.maxCoord.z << endl;
cout << "GPU : minCoord " << gA->cube.minCoord.x << "    " << gA->cube.minCoord.y << "   " << gA->cube.minCoord.z << endl;
cout << "CPU : maxCoord " << maxCoord.x << "    " << maxCoord.y << "   " << maxCoord.z << endl;
cout << "CPU : minCoord " << minCoord.x << "    " << minCoord.y << "   " << minCoord.z << endl;*/




/*std::cout << "Voxels listing - " << endl;
// Calcul de la grille de voxel utiles
// Pour chaque prismes, calculer les voxels intersectants le prismes et leur distance à la surface initiale
for (int i = 0; i < fromGeometry->listFaces.size(); i++)
{
//cout << " face " << i << " / " << temp->listFaces.size() << endl;
glm::vec3 s1 = fromGeometry->listVertex[fromGeometry->listFaces[i].s1];
glm::vec3 s2 = fromGeometry->listVertex[fromGeometry->listFaces[i].s2];
glm::vec3 s3 = fromGeometry->listVertex[fromGeometry->listFaces[i].s3];

glm::vec3 n1 = fromGeometry->listNormals[fromGeometry->listFaces[i].s1];
glm::vec3 n2 = fromGeometry->listNormals[fromGeometry->listFaces[i].s2];
glm::vec3 n3 = fromGeometry->listNormals[fromGeometry->listFaces[i].s3];

glm::vec4 t1 = fromGeometry->listTangents[fromGeometry->listFaces[i].s1];
glm::vec4 t2 = fromGeometry->listTangents[fromGeometry->listFaces[i].s2];
glm::vec4 t3 = fromGeometry->listTangents[fromGeometry->listFaces[i].s3];

glm::vec2 u1;
u1.x = fromGeometry->listCoords[fromGeometry->listFaces[i].s1].x;
u1.y = fromGeometry->listCoords[fromGeometry->listFaces[i].s1].y;
glm::vec2 u2 = glm::vec2(fromGeometry->listCoords[fromGeometry->listFaces[i].s2].x, fromGeometry->listCoords[fromGeometry->listFaces[i].s2].y);
glm::vec2 u3 = glm::vec2(fromGeometry->listCoords[fromGeometry->listFaces[i].s3].x, fromGeometry->listCoords[fromGeometry->listFaces[i].s3].y);


// Rajout : calcul du prisme et de sa boite englobante
glm::vec3 se1 = listExtrudedVertex[fromGeometry->listFaces[i].s1];
glm::vec3 se2 = listExtrudedVertex[fromGeometry->listFaces[i].s2];
glm::vec3 se3 = listExtrudedVertex[fromGeometry->listFaces[i].s3];

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

// Calcul du min et du max du prism dans le volume englobant subdiviser
//1ere étape en se rapporte au repère de la boite englobante (minCoord)
glm::vec3 minPoint = ((prismMin - minCoord) / (2.0f * halfVector)) * glm::vec3(nbVoxelPerAxis);
glm::vec3 maxPoint = ((prismMax - minCoord) / (2.0f * halfVector)) * glm::vec3(nbVoxelPerAxis);

// Reclamping dans [0:subdiv[
minPoint = clamp(glm::floor(minPoint), glm::vec3(0), glm::vec3(nbVoxelPerAxis - 1));
maxPoint = clamp(glm::floor(maxPoint), glm::vec3(0), glm::vec3(nbVoxelPerAxis - 1));

if (i < 10)
{
cout << minPoint.x << "   " << minPoint.y << "   " << minPoint.z << endl;
cout << maxPoint.x << "   " << maxPoint.y << "   " << maxPoint.z << endl;
}

// Pour chaque voxel du prism
for (int z_i = minPoint.z; z_i <= maxPoint.z; z_i++)
for (int y_i = minPoint.y; y_i <= maxPoint.y; y_i++)
for (int x_i = minPoint.x; x_i <= maxPoint.x; x_i++)
{
// Centre du voxel dans l'espace du cube
glm::vec3 coord_texture = ((glm::vec3(x_i, y_i, z_i) + 0.5f) / glm::vec3(nbVoxelPerAxis));
// Centre du voxel dans l'espace objet
glm::vec3 v_position = minCoord + coord_texture * halfVector * 2.0f;

// Projection sur la base du prisme
glm::vec3 baryCoords = glm::vec3(0, 0, 0);
glm::vec3 surface_position = MathUtils::triangleClosestPoint(v_position, s1, s2, s3, baryCoords);
glm::vec3 normale = baryCoords.x * n1 + baryCoords.y * n2 + baryCoords.z * n3;

// Si le voxel est assez proche de la surface, on le garde dans la liste
float voxelToSurface = glm::distance(surface_position, v_position);
float signToSurface = glm::dot(glm::normalize(v_position - surface_position), glm::normalize(normale));
//if (voxelToSurface <= shellHeight)// && signToSurface >= 0.0)
{

glm::vec4 tangente = baryCoords.x * t1 + baryCoords.y * t2 + baryCoords.z * t3;
glm::vec2 coord_surface = u1 * baryCoords.x + u2 * baryCoords.y + u3 * baryCoords.z;

this->push_back(
Voxel(
glm::vec4(v_position,0),
glm::vec4(normale,0),
tangente,
glm::vec4(coord_texture,0),
glm::vec4(coord_surface, 0, voxelToSurface)));
}
}
}*/


/*vector<Voxel> voxels;

std::cout << "Voxels listing - " << endl;
// Calcul de la grille de voxel utiles
// Pour chaque prismes, calculer les voxels intersectants le prismes et leur distance à la surface initiale
for (int i = 0; i < fromGeometry->listFaces.size(); i++)
{
//cout << " face " << i << " / " << temp->listFaces.size() << endl;
glm::vec3 s1 = fromGeometry->listVertex[fromGeometry->listFaces[i].s1];
glm::vec3 s2 = fromGeometry->listVertex[fromGeometry->listFaces[i].s2];
glm::vec3 s3 = fromGeometry->listVertex[fromGeometry->listFaces[i].s3];

glm::vec3 n1 = fromGeometry->listNormals[fromGeometry->listFaces[i].s1];
glm::vec3 n2 = fromGeometry->listNormals[fromGeometry->listFaces[i].s2];
glm::vec3 n3 = fromGeometry->listNormals[fromGeometry->listFaces[i].s3];

glm::vec4 t1 = fromGeometry->listTangents[fromGeometry->listFaces[i].s1];
glm::vec4 t2 = fromGeometry->listTangents[fromGeometry->listFaces[i].s2];
glm::vec4 t3 = fromGeometry->listTangents[fromGeometry->listFaces[i].s3];

glm::vec2 u1;
u1.x = fromGeometry->listCoords[fromGeometry->listFaces[i].s1].x;
u1.y = fromGeometry->listCoords[fromGeometry->listFaces[i].s1].y;
glm::vec2 u2 = glm::vec2(fromGeometry->listCoords[fromGeometry->listFaces[i].s2].x, fromGeometry->listCoords[fromGeometry->listFaces[i].s2].y);
glm::vec2 u3 = glm::vec2(fromGeometry->listCoords[fromGeometry->listFaces[i].s3].x, fromGeometry->listCoords[fromGeometry->listFaces[i].s3].y);


// Rajout : calcul du prisme et de sa boite englobante
glm::vec3 se1 = listExtrudedVertex[fromGeometry->listFaces[i].s1];
glm::vec3 se2 = listExtrudedVertex[fromGeometry->listFaces[i].s2];
glm::vec3 se3 = listExtrudedVertex[fromGeometry->listFaces[i].s3];

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

// Calcul du min et du max du prism dans le volume englobant subdiviser
//1ere étape en se rapporte au repère de la boite englobante (minCoord)
glm::vec3 minPoint = ((prismMin - minCoord) / (2.0f * halfVector)) * glm::vec3(nbVoxelPerAxis);
glm::vec3 maxPoint = ((prismMax - minCoord) / (2.0f * halfVector)) * glm::vec3(nbVoxelPerAxis);

// Reclamping dans [0:subdiv[
minPoint = clamp(glm::floor(minPoint), glm::vec3(0), glm::vec3(nbVoxelPerAxis - 1));
maxPoint = clamp(glm::floor(maxPoint), glm::vec3(0), glm::vec3(nbVoxelPerAxis - 1));*/


/*if (i < 10)
{
cout << "CPU min et max coord" << endl;
cout << minPoint.x << "   " << minPoint.y << "   " << minPoint.z << endl;
cout << maxPoint.x << "   " << maxPoint.y << "   " << maxPoint.z << endl;
}*/

// Pour chaque voxel du prism
/*for (int z_i = minPoint.z; z_i <= maxPoint.z; z_i++)
for (int y_i = minPoint.y; y_i <= maxPoint.y; y_i++)
for (int x_i = minPoint.x; x_i <= maxPoint.x; x_i++)
{
// Centre du voxel dans l'espace du cube
glm::vec3 coord_texture = ((glm::vec3(x_i, y_i, z_i) + 0.5f) / glm::vec3(nbVoxelPerAxis));
// Centre du voxel dans l'espace objet
glm::vec3 v_position = minCoord + coord_texture * halfVector * 2.0f;

// Projection sur la base du prisme
glm::vec3 baryCoords = glm::vec3(0, 0, 0);
glm::vec3 surface_position = MathUtils::triangleClosestPoint(v_position, s1, s2, s3, baryCoords);
glm::vec3 normale = baryCoords.x * n1 + baryCoords.y * n2 + baryCoords.z * n3;

// Si le voxel est assez proche de la surface, on le garde dans la liste
float voxelToSurface = glm::distance(surface_position, v_position);
float signToSurface = glm::dot(glm::normalize(v_position - surface_position), glm::normalize(normale));
//if (voxelToSurface <= shellHeight)// && signToSurface >= 0.0)
{

glm::vec4 tangente = baryCoords.x * t1 + baryCoords.y * t2 + baryCoords.z * t3;
glm::vec2 coord_surface = u1 * baryCoords.x + u2 * baryCoords.y + u3 * baryCoords.z;

voxels.push_back(
Voxel(
glm::vec4(v_position, 0),
glm::vec4(normale, 0),
tangente,
glm::vec4(coord_texture, 0),
glm::vec4(coord_surface, 0, voxelToSurface)));
}
}
}*/
/*voxelBuffer->bind();
VoxelArray* voxelArray = (VoxelArray*)malloc(sizeof(Voxel) * nTotal + sizeof(glm::ivec4));
voxelArray = (VoxelArray*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
cout << "affichage GPU coord surface voxel" << endl;
cout << "total voxcel " << nTotal << " " << voxels.size() << endl;
for (int i = 0; i < nTotal ; i++)
{
glm::vec4 cs = voxelArray->bufferContent[i].getVoxelData().coord_texture;
glm::vec4 cs2 = voxels[i].getVoxelData().coord_texture;
if (cs.x != cs2.x || cs.y != cs2.y || cs.z != cs2.z)
{
cout << cs.x << "  " << cs.y << "  " << cs.z << "  " << cs.w << endl;
cout << cs2.x << "  " << cs2.y << "  " << cs2.z << "  " << cs2.w << endl;
}
glm::vec4 cs3 = voxelArray->bufferContent[i].getVoxelData().coord_surface;
glm::vec4 cs4 = voxels[i].getVoxelData().coord_surface;
if (cs3.w != cs4.w)
{
//cout << cs3.w << "  " << cs4.w << endl;
}

}
glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);*/
/*cout << "cpu voxels" << endl;
for (int i = nTotal - 1; i > nTotal - 40; i--)
{
cout << voxels[i].getVoxelData().coord_surface.x << "  " << voxels[i].getVoxelData().coord_surface.y << "  " << voxels[i].getVoxelData().coord_surface.z << "  " << voxels[i].getVoxelData().coord_surface.w << endl;
}*/

/*std::vector<bool> flagVoxelToDelete(voxels.size());
for (int i = 0; i < voxels.size(); i++)
{
flagVoxelToDelete[i] = false;
}
//// C'est là que le code est extrèmement lent (double parcours)
std::cout << "Voxels Reduction - ";
for (int i = 0; i < 1000; i++)
{
VoxelData* v1 = voxels.at(i).getAdressOfData();
for (int j = i + 1; j < voxels.size(); j++)
{
if (i != j)
{
VoxelData* v2 = voxels.at(j).getAdressOfData();
// Si les 2 voxels sont au même endroit
if (glm::all(glm::equal(glm::vec3(v1->coord_texture), glm::vec3(v2->coord_texture))))
{
// on flag le voxel le plus éloigné de la surface
if (v1->coord_surface.w >= v2->coord_surface.w)
{
flagVoxelToDelete[i] = true;
}
}
}
}
}*/



/*deleteBuffer->bind();
deleteArray* gdel = (deleteArray*)malloc(deleteBufferSize);
gdel = (deleteArray*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
cout << "affichage delete GPU" << endl;
for (int i = 0; i < 1000; i++)
{
if (gdel[i].del.x == 0)
{
//if (gdel[i].del.x)
cout << gdel[i].del.x << endl;
cout << i << endl;
}
}

glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);*/

/*std::vector<Voxel> tempCopy;
for (int i = 0; i < this->size(); i++)
{
if (!flagVoxelToDelete[i]) tempCopy.push_back(this->at(i));
}
std::cout << "done" << std::endl;
this->swap(tempCopy);*/