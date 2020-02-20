#ifndef SKELETON_MESH_H
#define SKELETON_MESH_H


#include "Skeleton.h"
#include "Vertex.h"


class SkeletonMesh
{
public:
	SkeletonMesh(glm::mat4 rootTransform = glm::mat4(1.0));

	std::vector<Vertex> m_vertices = {};

	std::vector<uint32_t> m_indices = {};

	glm::mat4 m_rootTransform;

	Skeleton m_skeleton;

	void addBoneToVertex(unsigned int boneID, unsigned int vertexID, float weights);


	void playAnimation();

};



void printModelStats(const aiScene* scene);


SkeletonMesh loadSkeletonMesh(const std::string& filepath, const std::string& rootBoneName);


void printSkeletonBones(Skeleton sk);


#endif