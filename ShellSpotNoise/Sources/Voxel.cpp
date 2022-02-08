
#include "Voxel.h"
#include <algorithm>
#include "myMathUtils.hpp"
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
	const glm::vec4 & position, 
	const glm::vec4 & normale, 
	const glm::vec4 & tangente, 
	const glm::vec4 & coord_texture, 
	const glm::vec4 & coord_surface)
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
	const glm::vec4 & position)
{
	data.position = position;
}

void Voxel::changeNormale(
	const glm::vec4 & normale)
{
	data.normale = normale;
}

void Voxel::changeTangente(
	const glm::vec4 & tangente)
{
	data.tangente = tangente;
}

void Voxel::changeCoord_texture(
	const glm::vec4 & coord_texture)
{
	data.coord_texture = coord_texture;
}


void Voxel::changeCoord_surface(
	const glm::vec4 & coord_surface)
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
	computeDone = false;
	this->compute(fromGeometry,subdivision,shellHeight);
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
		std::cout << "done" << std::endl;;
		// Transformation de la bounding box en bounding cube :
		glm::vec3 center = (minCoord + maxCoord) * glm::vec3(0.5);
		glm::vec3 halfVector = (maxCoord - minCoord ) * glm::vec3(0.5);
		float halfSize = max(max(halfVector.x, halfVector.y), halfVector.z);
		halfVector = glm::vec3(halfSize);
		minCoord = center - halfVector;
		maxCoord = center + halfVector;
		
		int nbVoxelPerAxis = pow(2.0, subdivision);

		voxelHalfSize = halfSize / (float)nbVoxelPerAxis;

		AABB = new GeometricBoundingBox(NULL);
		AABB->compute(minCoord, maxCoord);
				

		std::cout << "Voxels listing - ";
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
			glm::vec3 minPoint = ((prismMin - minCoord) / (2.0f * halfVector)) * glm::vec3(nbVoxelPerAxis);
			glm::vec3 maxPoint = ((prismMax - minCoord) / (2.0f * halfVector)) * glm::vec3(nbVoxelPerAxis);
			
			// Reclamping dans [0:subdiv[
			minPoint = clamp(glm::floor(minPoint), glm::vec3(0), glm::vec3(nbVoxelPerAxis - 1));
			maxPoint = clamp(glm::floor(maxPoint), glm::vec3(0), glm::vec3(nbVoxelPerAxis - 1));

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
		}
		std::cout << "done" << std::endl;;

		
		//std::vector<bool> flagVoxelToDelete(this->size());
		//for (int i = 0; i < this->size(); i++)
		//{
		//	flagVoxelToDelete[i] = false;
		//}
		//// C'est là que le code est extrèmement lent (double parcours)
		//std::cout << "Voxels Reduction - ";
		//for (int i = 0; i < this->size() - 1; i++)
		//{
		//	VoxelData* v1 = this->at(i).getAdressOfData();
		//	for (int j = i+1; j < this->size(); j++)
		//	{
		//		VoxelData* v2 = this->at(j).getAdressOfData();
		//		// Si les 2 voxels sont au même endroit
		//		if (glm::all(glm::equal(glm::vec3(v1->coord_texture), glm::vec3(v2->coord_texture))))
		//		{
		//			// on flag le voxel le plus éloigné de la surface
		//			if (v1->coord_surface.w < v2->coord_surface.w) flagVoxelToDelete[j] = true;
		//			else flagVoxelToDelete[i] = true;
		//		}
		//	}
		//}
		//
		//std::vector<Voxel> tempCopy;
		//for (int i = 0; i < this->size(); i++)
		//{
		//	if (!flagVoxelToDelete[i]) tempCopy.push_back(this->at(i));
		//}
		//std::cout << "done" << std::endl;;
		//this->swap(tempCopy);
		
		

		computeDone = true;
	}
}

VoxelVector::VoxelVector(const char* fileName)
{
	loadFromFile(fileName);
}

GeometricBoundingBox* VoxelVector::getGeometricBoundingBox()
{
	return AABB;
}

void VoxelVector::saveInFile(const char* fileName)
{
	///TODO
}

void VoxelVector::loadFromFile(const char* fileName)
{
	///TODO
}


GPUVoxelVector::GPUVoxelVector()
:VoxelVector()
{

}

GPUVoxelVector::GPUVoxelVector(
	GeometricModel* fromGeometry,
	int SSBO_binding,
	int subdivision,
	float shellHeight)
	: VoxelVector(fromGeometry, subdivision, shellHeight)
{
	//createData(SSBO_binding);
}

GPUVoxelVector::GPUVoxelVector(const char* fileName, int SSBO_binding)
	: VoxelVector(fileName)
{
	//createData(SSBO_binding);
	loadDataToGPU(SSBO_binding);
}

void GPUVoxelVector::createData(int SSBO_binding)
{
	try{
		// Création du buffer de voxel
		voxelBuffer = new GPUBuffer("VoxelBuffer");
		int voxelBufferSize = sizeof(glm::vec4) + sizeof(Voxel)* this->size();

		voxelBuffer->create(voxelBufferSize, GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW);
		voxelBuffer->setBindingPoint(4);

		voxelBuffer->bind();

		// Ptr to VoxelArray structure in GPU
		VoxelArray* ptr = (VoxelArray*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);

		ptr->size.x = float(this->size());
		ptr->size.y = this->voxelHalfSize;
		ptr->size.z = 0;
		ptr->size.w = 0;
		memcpy(ptr->bufferContent, &(this->at(0)), sizeof(Voxel)* this->size());

		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);



		flagBuffer = new GPUBuffer("flagBuffer");
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
		
		computeDone = true;


	}
	catch (const std::exception & e)
	{
		std::cout << e.what() << std::endl;
	}

	if (this->size() > 0 & computeDone)
	{
		GPU_AABB = new BoundingBoxModelGL(AABB);
		loadDataToGPU(SSBO_binding);
	}
}

void GPUVoxelVector::loadDataToGPU(int SSBO_binding)
{

	//if (computeDone)
	{
	
		voxelBuffer = new GPUBuffer("VoxelBuffer");
		int voxelBufferSize = sizeof(glm::vec4) + sizeof(Voxel)* this->size();
	
		voxelBuffer->create(voxelBufferSize, GL_SHADER_STORAGE_BUFFER, GL_STATIC_DRAW);
		voxelBuffer->setBindingPoint(SSBO_binding);

		voxelBuffer->bind();
	
		// Ptr to VoxelArray structure in GPU
		VoxelArray* ptr = (VoxelArray*)glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);

		ptr->size.x = float(this->size());
		ptr->size.y = this->voxelHalfSize;
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