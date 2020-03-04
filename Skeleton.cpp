#include "Skeleton.h"
#include "Global.h"


bool Skeleton::operator==(const Skeleton& other)
{
	std::set<std::string> othersBoneNames;
	for (auto bone : other.m_bones)
	{
		othersBoneNames.insert(bone.first);
	}
	for (auto bone : this->m_bones)
	{
		if (othersBoneNames.count(bone.first) == 0)
		{
			std::cout << bone.first << std::endl;
			return false;
		}
		othersBoneNames.erase(bone.first);
	}
	return othersBoneNames.empty();
}

bool Skeleton::addSocket(std::string parent, std::string socketName)
{
	if (m_bones.count(parent) == 0)
	{
		std::cerr << "parent not exists" << std::endl;
		return false;
	}

	Bone* pParent = &m_bones.at(parent);

	Socket socket;
	
	socket.m_boneId = (unsigned int) (m_bones.size() + m_sockets.size());
	socket.m_name = socketName;
	socket.m_offset = pParent->m_offset;
	socket.m_parent = parent;
	socket.m_bindPoseTransformToParent = glm::mat4(1.0);

	m_sockets.insert(std::pair(socketName, socket));
	return true;
}

bool Skeleton::deleteSocket(std::string name)
{
	if (m_sockets.count(name) == 0)
	{
		std::cerr << "no such socket exists" << std::endl;
		return false;
	}

	m_sockets.erase(name);
	return true;
}

void Skeleton::printBones()
{
	for (auto bone : m_bones)
	{
		std::cout << bone.first << std::endl;
	}
}

glm::mat4 mat4_cast(const aiMatrix4x4& ai)
{
	glm::mat4 mat;

	mat[0][0] = ai.a1; mat[1][0] = ai.a2; mat[2][0] = ai.a3; mat[3][0] = ai.a4;
	mat[0][1] = ai.b1; mat[1][1] = ai.b2; mat[2][1] = ai.b3; mat[3][1] = ai.b4;
	mat[0][2] = ai.c1; mat[1][2] = ai.c2; mat[2][2] = ai.c3; mat[3][2] = ai.c4;
	mat[0][3] = ai.d1; mat[1][3] = ai.d2; mat[2][3] = ai.d3; mat[3][3] = ai.d4;

	return mat;;
}

void traceRootBone(aiNode* curNode, aiNode*& pRootNode, const std::string& rootBoneName)
{
	if (strcmp(curNode->mName.C_Str(), rootBoneName.c_str()) == 0)
	{
		pRootNode = curNode;
		return;
	}

	if (curNode->mNumChildren == 0)
	{
		return;
	}

	for (unsigned int i = 0; i < curNode->mNumChildren; i++)
	{
		traceRootBone(curNode->mChildren[i], pRootNode, rootBoneName);
	}
}

Skeleton* Skeleton::extractSkeletonFromAnimFile(const aiScene* scene, const std::string& rootBoneName)
{
	Skeleton* skeleton = new Skeleton();

	std::stack<aiNode*> nodes;

	aiNode* pRootBone = nullptr;

	traceRootBone(scene->mRootNode, pRootBone, rootBoneName);

	if (pRootBone == nullptr)
	{
		throw std::runtime_error("");
	}

	unsigned int currentID = 2;
	Bone rootBone(pRootBone->mName.C_Str(), "", glm::mat4(1.0));
	rootBone.m_boneId = currentID;
	rootBone.m_lengthToParent = 0.0f;

	for (unsigned int i = 0; i < pRootBone->mNumChildren; i++)
	{
		rootBone.m_children.push_back(pRootBone->mChildren[i]->mName.C_Str());
	}
	currentID++;


	skeleton->m_bones.insert(std::pair(pRootBone->mName.C_Str(), rootBone));

	nodes.push(pRootBone);

	while (!nodes.empty())
	{
		aiNode* currentNode = nodes.top();
		nodes.pop();
		for (unsigned int i = 0; i < currentNode->mNumChildren; i++)
		{
			aiNode* child = currentNode->mChildren[i];
			Bone bone(child->mName.C_Str(), currentNode->mName.C_Str(), mat4_cast(child->mTransformation));
			bone.m_lengthToParent = glm::length(bone.m_bindPoseTransformToParent * glm::vec4(0, 0, 0, 1));
			for (unsigned int j = 0; j < child->mNumChildren; j++)
			{
				bone.m_children.push_back(child->mChildren[j]->mName.C_Str());
			}
			bone.m_boneId = currentID;
			currentID++;
			skeleton->m_bones.insert(std::pair(child->mName.C_Str(), bone));
			nodes.push(child);
		}
	}

	if (scene->mNumMeshes > 0)
	{
		for (unsigned int i = 0; i < scene->mMeshes[0]->mNumBones; i++)
		{
			aiBone* curBone = scene->mMeshes[0]->mBones[i];
			std::string boneName = curBone->mName.C_Str();

			auto worldTransformation = FBX_Import_Mesh_Root_Transformation * glm::inverse(mat4_cast(curBone->mOffsetMatrix));
			auto worldTransformationInv = glm::inverse(worldTransformation);
			skeleton->m_bones.at(boneName).m_offset = worldTransformationInv;
			skeleton->m_bones.at(boneName).m_bindPoseWorldTransform = worldTransformation;
		}
	}


	return skeleton;
}

Bone::Bone()
{
	m_lengthToParent = 0;
	m_children = {};
	m_worldCoord = glm::mat4(1.0);
}

Bone::Bone(std::string boneName, std::string parent, glm::mat4 transformation)
{
	m_name = boneName;
	m_parent = parent;
	m_bindPoseTransformToParent = transformation;
	m_children = {};
	m_worldCoord = glm::mat4(1.0);
}

Socket::Socket()
{
}

Socket::~Socket()
{
}
