#pragma once

#include <iostream>
#include <string>
#include <unordered_map>
#include <set>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>         
#include <assimp/postprocess.h> 
#include <stack>

#include "Math.h"

glm::mat4 mat4_cast(const aiMatrix4x4& ai);



struct Bone
{
	std::string m_name;

	std::string m_parent;

	unsigned int m_boneId;

	glm::mat4 m_offset;

	glm::mat4 m_bindPoseWorldTransform;

	glm::mat4 m_bindPoseTransformToParent;

	float m_lengthToParent;

	glm::mat4 m_worldCoord;

	std::vector<std::string> m_children;

	Bone();

	Bone(std::string boneName, std::string parent, glm::mat4 transformation);

};

struct Socket
{
	Socket(glm::mat4 coord, std::string name, std::string parent);
	~Socket();
	glm::mat4 m_coordInParent;
	std::string m_socketName;
	std::string m_parent;
};

class Skeleton
{
public:
	static Skeleton* extractSkeletonFromAnimFile(const aiScene* scene, const std::string& rootBoneName);

	Socket* getSocket(std::string name);

	glm::mat4 getSocketLocation(std::string socketName, const std::vector<glm::mat4>& boneT);

	std::unordered_map<std::string, Bone> m_bones;
	
	bool operator == (const Skeleton& other);

	bool addSocket(std::string parent, std::string socketName, glm::mat4 transformToParent);

	bool deleteSocket(std::string name);

	void printBones();


private:
	std::unordered_map<std::string, Socket> m_sockets;

};

void traceRootBone(aiNode* curNode, aiNode*& pRootNode, const std::string& rootBoneName);
