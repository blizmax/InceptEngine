#ifndef SKELETON_MESH_H
#define SKELETON_MESH_H

#include <iostream>
#include <string>
#include "Vertex.h"

class Renderer;
class Animation;
struct VertexBuffer;
struct IndexBuffer;
struct Texture;
struct TransformationBuffer;
struct DataDescription;
class Skeleton;

class SkeletonMesh
{
public:
	SkeletonMesh(Renderer* renderer, const std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::string texturePath, Skeleton* skeleton, glm::mat4 rootTransform = glm::mat4(1.0));

	~SkeletonMesh();

	//std::vector<Vertex> m_vertices = {};

	//std::vector<uint32_t> m_indices = {};

	
	static SkeletonMesh* loadSkeletonMesh(Renderer* renderer, const std::string& filepath, std::string texturePath, const std::string& rootBoneName);
	
	//void setProperties(Renderer* renderer, std::vector<Vertex>* vertices, std::vector<uint32_t>* indices, std::string texturePath);

	VkBuffer* getMeshVerticesBuffer();

	VkBuffer getMeshIndicesBuffer();

	uint32_t getNumIndices();

	Skeleton* getSkeleton();

	void playAnimation();

	VkDescriptorSet* getDescritorset(int i);

	void initializeTransformationBuffer(Renderer* renderer, const std::vector<glm::mat4>& transformations);

	void updateTransformationBuffer(Renderer* renderer, const std::vector<glm::mat4>& transformations);

private:
	static void addBoneToVertex(std::vector<Vertex>& vertices, unsigned int boneID, unsigned int vertexID, float weights);

private:
	glm::mat4 m_rootTransform;

	VertexBuffer* m_vertexBuffer;

	uint32_t n_indices;

	IndexBuffer* m_indexBuffer;

	Texture* m_texture;

	TransformationBuffer* m_tBuffer;

	DataDescription* m_dataDesc;

	Skeleton* m_skeleton;

	Animation* m_currentAnimation;

};






#endif