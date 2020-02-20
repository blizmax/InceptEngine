#ifndef SKELETON_H
#define SKELETON_H

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

	glm::mat4 m_transformToParent;


	Bone();

	Bone(std::string boneName, std::string parent, glm::mat4 transformation);

};

struct Socket : public Bone
{
	Socket();
	~Socket();
};

class Skeleton
{
public:
	std::unordered_map<std::string, Bone> m_bones;
	
	bool operator == (const Skeleton& other);

	bool addSocket(std::string parent, std::string socketName);

	bool deleteSocket(std::string name);

private:
	std::unordered_map<std::string, Socket> m_sockets;
};

void traceRootBone(aiNode* curNode, aiNode*& pRootNode, const std::string& rootBoneName);


Skeleton extractSkeletonFromAnimFile(const aiScene* scene, const std::string& rootBoneName);

#endif
