#include "SkeletonMesh.h"
#include "Renderer.h"
#include "Animation.h"
#include "Skeleton.h"

SkeletonMesh::SkeletonMesh(Renderer* renderer, const std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::string texturePath, Skeleton* skeleton, glm::mat4 rootTransform)
{
	m_rootTransform = rootTransform;

	m_vertexBuffer = renderer->createVertexBuffer(vertices);

	m_indexBuffer = renderer->createIndexBuffer(indices);

	n_indices = static_cast<uint32_t>(indices.size());

	m_texture = renderer->createTexture(texturePath);

	m_skeleton = skeleton;

	m_tBuffer = renderer->createTransformationBuffer(); //just allocate the buffer, no data binding now

	m_dataDesc = renderer->createDataDescription(*m_tBuffer, *m_texture);

	m_currentAnimation = nullptr;
}

SkeletonMesh::~SkeletonMesh()
{
	if (m_indexBuffer != nullptr)
	{
		delete m_indexBuffer;
	}
	if (m_vertexBuffer != nullptr)
	{
		delete m_vertexBuffer;
	}
	if (m_texture != nullptr)
	{
		delete m_texture;
	}
	if (m_tBuffer != nullptr)
	{
		delete m_tBuffer;
	}
	if (m_dataDesc != nullptr)
	{
		delete m_dataDesc;
	}
	if (m_currentAnimation != nullptr)
	{
		delete m_currentAnimation;
	}
	if (m_skeleton != nullptr)
	{
		delete m_skeleton;
	}
}

void SkeletonMesh::addBoneToVertex(std::vector<Vertex>& vertices, unsigned int boneID, unsigned int vertexID, float weights)
{
	if (vertices[vertexID].affectedBonesID[0] == 0)
	{
		vertices[vertexID].boneWeights[0] = 0.0f;
	}

	float min = 1.0f;
	unsigned int minIdx = 0;
	for (unsigned int i = 0; i < MAX_BONE_PER_VERTEX; i++)
	{
		if (vertices[vertexID].boneWeights[i] < min)
		{
			min = vertices[vertexID].boneWeights[i];
			minIdx = i;
		}
	}

	vertices[vertexID].boneWeights[minIdx] = weights;
	vertices[vertexID].affectedBonesID[minIdx] = boneID;
}
/*
void SkeletonMesh::setProperties(Renderer* renderer, std::vector<Vertex>* vertices, std::vector<uint32_t>* indices, std::string texturePath)
{
	if (renderer == nullptr)
	{
		throw std::runtime_error("");
	}

	if (vertices != nullptr)
	{
		if (m_vertexBuffer != nullptr)
		{
			delete m_vertexBuffer;
		}
		m_vertexBuffer = renderer->createVertexBuffer(*vertices);
		n_indices = static_cast<uint32_t>(indices->size());
	}

	if (indices != nullptr)
	{
		if (m_indexBuffer != nullptr)
		{
			delete m_indexBuffer;
		}
		m_indexBuffer = renderer->createIndexBuffer(*indices);
	}
}*/

VkBuffer* SkeletonMesh::getMeshVerticesBuffer()
{
	return &m_vertexBuffer->m_vertexBuffer;
}

VkBuffer SkeletonMesh::getMeshIndicesBuffer()
{
	return m_indexBuffer->m_indexBuffer;
}

uint32_t SkeletonMesh::getNumIndices()
{
	return n_indices;
}

Skeleton* SkeletonMesh::getSkeleton()
{
	return m_skeleton;
}

void SkeletonMesh::playAnimation()
{
}

VkDescriptorSet* SkeletonMesh::getDescritorset(int i)
{
	return &m_dataDesc->m_descriptorSet[i];
}

void SkeletonMesh::initializeTransformationBuffer(Renderer* renderer, const std::vector<glm::mat4>& transformations)
{
	if (m_tBuffer == nullptr)
	{
		throw std::runtime_error("");
	}
	renderer->initializeTransformationBuffer(*m_tBuffer, transformations);
}

void SkeletonMesh::updateTransformationBuffer(Renderer* renderer, const std::vector<glm::mat4>& transformations)
{
	renderer->updateTransformationBuffer(*m_tBuffer, transformations);
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

SkeletonMesh* SkeletonMesh::loadSkeletonMesh(Renderer* renderer, const std::string& filepath, std::string texturePath, const std::string& rootBoneName)
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


	aiMesh* mesh = scene->mMeshes[0];

	std::vector<Vertex> vertices;
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

		vertices.push_back(v);
	}


	std::vector<uint32_t> indices;
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace* face = &mesh->mFaces[i];
		assert(face->mNumIndices == 3);
		indices.push_back((uint32_t)face->mIndices[0]);
		indices.push_back((uint32_t)face->mIndices[1]);
		indices.push_back((uint32_t)face->mIndices[2]);
	}
	assert(indices.size() % 3 == 0);

	Skeleton* skeleton = Skeleton::extractSkeletonFromAnimFile(scene, rootBoneName);

	for (unsigned int i = 0; i < mesh->mNumBones; i++)
	{
		if (skeleton->m_bones.count(mesh->mBones[i]->mName.C_Str()) == 0)
		{
			throw std::runtime_error("");
		}
	}

	for (unsigned int i = 0; i < mesh->mNumBones; i++)
	{
		std::string curBoneName = mesh->mBones[i]->mName.C_Str();
		unsigned int curBoneID = skeleton->m_bones.at(curBoneName).m_boneId;
		for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++)
		{
			float weight = mesh->mBones[i]->mWeights[j].mWeight;
			unsigned int affectedVertexId = mesh->mBones[i]->mWeights[j].mVertexId;
			addBoneToVertex(vertices, curBoneID, affectedVertexId, weight);
		}
	}

	
	return new SkeletonMesh(renderer, vertices, indices, texturePath, skeleton);
}


