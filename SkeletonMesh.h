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
struct UniformBuffer;
struct DataDescription;
class Skeleton;
struct Light;

class SkeletonMesh
{
public:
	SkeletonMesh(Renderer* renderer, const std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, std::string texturePath, Skeleton* skeleton);

	~SkeletonMesh();

	//std::vector<Vertex> m_vertices = {};

	//std::vector<uint32_t> m_indices = {};

	
	static SkeletonMesh* loadSkeletonMesh(Renderer* renderer, const std::string& filepath, std::string texturePath, const std::string& rootBoneName, bool importAllMesh);
	

	VkBuffer* getMeshVerticesBuffer();

	VkBuffer getMeshIndicesBuffer();

	uint32_t getNumIndices();

	Skeleton* getSkeleton();

	void playAnimation();

	VkDescriptorSet* getDescritorset(int i);

	void initializeUniformBuffer(Renderer* renderer, const std::vector<glm::mat4>& transformations, Light* light);

	void updateUniformBuffer(Renderer* renderer, const std::vector<glm::mat4>& transformations, Light* light);


private:
	static void addBoneToVertex(std::vector<Vertex>& vertices, unsigned int boneID, unsigned int vertexID, float weights);

private:

	VertexBuffer* m_vertexBuffer;

	uint32_t n_indices;

	IndexBuffer* m_indexBuffer;

	Texture* m_texture;

	UniformBuffer* m_uBuffer;

	DataDescription* m_dataDesc;

	Skeleton* m_skeleton;

	Animation* m_currentAnimation;

};






#endif