#include "SkeletonMesh.h"

SkeletonMesh::SkeletonMesh(glm::mat4 rootTransform)
{
	m_rootTransform = rootTransform;
}

void SkeletonMesh::addBoneToVertex(unsigned int boneID, unsigned int vertexID, float weights)
{
	if (m_vertices[vertexID].affectedBonesID[0] == 0)
	{
		m_vertices[vertexID].boneWeights[0] = 0.0f;
	}

	float min = 1.0f;
	unsigned int minIdx = 0;
	for (unsigned int i = 0; i < MAX_BONE_PER_VERTEX; i++)
	{
		if (m_vertices[vertexID].boneWeights[i] < min)
		{
			min = m_vertices[vertexID].boneWeights[i];
			minIdx = i;
		}
	}



	m_vertices[vertexID].boneWeights[minIdx] = weights;
	m_vertices[vertexID].affectedBonesID[minIdx] = boneID;
}

void SkeletonMesh::playAnimation()
{
}

void printModelStats(const aiScene* scene)
{
	std::cout << "The model has " << scene->mNumMeshes << " meshes" << std::endl;
	std::cout << "The model has " << scene->mNumAnimations << " anims" << std::endl;
	std::cout << "The model has " << scene->mNumCameras << " cameras" << std::endl;
	std::cout << "The model has " << scene->mNumLights << " lights" << std::endl;
	std::cout << "The model has " << scene->mNumMaterials << " materials" << std::endl;
	std::cout << "The model has " << scene->mNumTextures << " textures" << std::endl;

}

SkeletonMesh loadSkeletonMesh(const std::string& filepath, const std::string& rootBoneName)
{

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(filepath,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);


	if (!scene)
	{
		throw std::runtime_error("");
	}

	printModelStats(scene);


	if (!scene->HasMeshes())
	{
		throw std::runtime_error("");
	}


	SkeletonMesh skMesh;
	aiMesh* mesh = scene->mMeshes[0];


	bool hasTextureCoords = mesh->HasTextureCoords(0);

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex v;
		v.position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z, 1 };
		
		if (mesh->HasVertexColors(i))
		{
			v.color = { mesh->mColors[i][0].r, mesh->mColors[i][0].g, mesh->mColors[i][0].b, 1.0 };
			std::cout << i << std::endl;
		}
		else
		{
			v.color = { 0.0, 0.0, 0.0, 1.0 };
		}

		if (hasTextureCoords)
		{
			v.texCoord = { mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y, mesh->mTextureCoords[0][i].z };
		}

		skMesh.m_vertices.push_back(v);
	}



	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace* face = &mesh->mFaces[i];
		assert(face->mNumIndices == 3);
		skMesh.m_indices.push_back((uint32_t)face->mIndices[0]);
		skMesh.m_indices.push_back((uint32_t)face->mIndices[1]);
		skMesh.m_indices.push_back((uint32_t)face->mIndices[2]);
	}
	assert(skMesh.m_indices.size() % 3 == 0);

	skMesh.m_skeleton = extractSkeletonFromAnimFile(scene, rootBoneName);

	for (unsigned int i = 0; i < mesh->mNumBones; i++)
	{
		if (skMesh.m_skeleton.m_bones.count(mesh->mBones[i]->mName.C_Str()) == 0)
		{
			throw std::runtime_error("");
		}
	}

	for (unsigned int i = 0; i < mesh->mNumBones; i++)
	{
		std::string curBoneName = mesh->mBones[i]->mName.C_Str();
		unsigned int curBoneID = skMesh.m_skeleton.m_bones.at(curBoneName).m_boneId;
		for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++)
		{
			float weight = mesh->mBones[i]->mWeights[j].mWeight;
			unsigned int affectedVertexId = mesh->mBones[i]->mWeights[j].mVertexId;
			skMesh.addBoneToVertex(curBoneID, affectedVertexId, weight);
		}
	}


	return skMesh;
}

void printSkeletonBones(Skeleton sk)
{
	for (auto bone : sk.m_bones)
	{
		std::cout << bone.first << std::endl;
	}
}
