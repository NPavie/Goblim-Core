#include "AssimpLoader.h"
#include "Engine/Base/GeometricModel.h"

#include <stdexcept>
#ifdef USE_ASSIMP

AssimpLoader::AssimpLoader()
{
}
AssimpLoader::~AssimpLoader()
{
}
bool AssimpLoader::loadModel(string filename,GeometricModel *model)
{
	// Create an instance of the Importer class
	Assimp::Importer importer;

	const aiScene* scene = importer.ReadFile( filename,
			aiProcess_CalcTangentSpace |
			aiProcess_Triangulate |
			aiProcess_GenSmoothNormals |
			aiProcess_OptimizeMeshes |
			aiProcess_JoinIdenticalVertices |
			aiProcess_SortByPType);

	// If the import failed, report it
	if( !scene)
		throw std::logic_error(string("ERROR : Assimp Geometric Model Loader: ") + filename + string(" could not be opened.\n") + string(importer.GetErrorString()));

	// GetMeshes
	aiMesh **meshes = scene->mMeshes;

	for (unsigned int i = 0;i <scene->mNumMeshes;i++)	// Group everithing into one mesh
	{
		loadFromMesh(meshes[i],model);
	}

	return true;

}

bool AssimpLoader::loadFromMesh(aiMesh* mesh, GeometricModel *model)
{
	int baseVertex = model->nb_vertex;
	int baseFaces = model->nb_faces;

	model->nb_vertex += mesh->mNumVertices;
	model->nb_faces += mesh->mNumFaces;
	for(unsigned int i =0;i <  mesh->mNumVertices;i++)
	{
		model->listVertex.push_back(glm::vec3(mesh->mVertices[i].x,mesh->mVertices[i].y,mesh->mVertices[i].z));
		// Start of border vertex test : all vertex are considered as non border vertex :
		// Only border vertex will have neigbours computed depending on their belonging to border edges
		model->listBorderNeighbours.push_back(glm::vec3(-1.0));
	}
	if (mesh->HasNormals())
		for(unsigned int i =0;i <  mesh->mNumVertices;i++)
		{
			glm::vec3 v(mesh->mNormals[i].x,mesh->mNormals[i].y,mesh->mNormals[i].z);
			model->listNormals.push_back(v);
		}

	if (mesh->HasFaces())
	{
		
		for(unsigned int i =0;i <   mesh->mNumFaces;i++)
		{
			Face f;
			f.s1 = baseVertex+mesh->mFaces[i].mIndices[0];
			f.s2 = baseVertex+mesh->mFaces[i].mIndices[1];
			f.s3 = baseVertex+mesh->mFaces[i].mIndices[2];
			model->listFaces.push_back(f);
			
			// ------- Addon : Edges treatment : testing if an edge is on the border of the mesh for open mesh
			
			// Current edge composed of 2 of the 3 face vertex
			Arete currentEdge;
			// to avoid symetry in the data set, all edges are considered as (minIndex, maxIndex) of the vertices index
			currentEdge.s1 = min(f.s1,f.s2);
			currentEdge.s2 = max(f.s1,f.s2);

			// First edge of the face : test to find if the edges has already been defined
			std::map<Arete,int,Arete>::iterator edge_i = model->listEdges.find(currentEdge);
			if(edge_i != model->listEdges.end())
			{
				// Edge already defined : we increment the face counter
				edge_i->second += 1;
			}
			else
			{
				// First apparition : we add the edge in the list and set its face counter to 1
				model->listEdges[currentEdge] = 1;
			}

			// Same instructions as before on the second edge
			currentEdge.s1 = min(f.s2,f.s3);
			currentEdge.s2 = max(f.s2,f.s3);
			edge_i = model->listEdges.find(currentEdge);
			if(edge_i != model->listEdges.end())
			{
				edge_i->second += 1;
			}
			else
			{
				model->listEdges[currentEdge] = 1;
			}

			// Same instructions as before on the third edge
			currentEdge.s1 = min(f.s3,f.s1);
			currentEdge.s2 = max(f.s3,f.s1);
			edge_i = model->listEdges.find(currentEdge);
			if(edge_i != model->listEdges.end())
			{
				edge_i->second += 1;
			}
			else
			{
				model->listEdges[currentEdge] = 1;
			}

		}

		
		// So for now we have an edge list with the number of appearance of each edge in this list.
		// By definition, a border edge belongs to only one face, so the edge has to appear only one time in the list
		
		std::map<Arete,int,Arete>::iterator edge_i;
		for(edge_i = model->listEdges.begin(); edge_i != model->listEdges.end(); edge_i++)
		{
			// If the edge appears only once in the list, 
			if(edge_i->second == 1.0)
			{
				// keeping track of the vertices ID (only if they are on the border
				model->listBorderNeighbours[edge_i->first.s1].x = edge_i->first.s1; 
				model->listBorderNeighbours[edge_i->first.s2].x = edge_i->first.s2; 

				// hypothesis : a border vertex has only two border vertices
				// storing neighbours of the edge first vertex
				if (model->listBorderNeighbours[edge_i->first.s1].y == -1.0)	// If no data have been set before for s1
					model->listBorderNeighbours[edge_i->first.s1].y = edge_i->first.s2;
				else
					model->listBorderNeighbours[edge_i->first.s1].z = edge_i->first.s2;

				// storing neighbours of the edge second vertex
				if (model->listBorderNeighbours[edge_i->first.s2].y == -1.0) // If no data have been set before for s2
					model->listBorderNeighbours[edge_i->first.s2].y = edge_i->first.s1;
				else
					model->listBorderNeighbours[edge_i->first.s2].z = edge_i->first.s1;
					
			}
		}

	}
	if (mesh->HasTextureCoords(0))
		for(unsigned int i =0;i <mesh->mNumVertices;i++)
		{
			glm::vec3 v(mesh->mTextureCoords[0][i].x,mesh->mTextureCoords[0][i].y,mesh->mTextureCoords[0][i].z);
			model->listCoords.push_back(v);
		}
	if (mesh->HasTangentsAndBitangents())
		for(unsigned int i =0;i <  mesh->mNumVertices;i++)
		{
			glm::vec4 v(mesh->mTangents[i].x,mesh->mTangents[i].y,mesh->mTangents[i].z,1.0f);
			model->listTangents.push_back(v);
		}

		model->createBoundingBox();
	return(true);
}


#endif
