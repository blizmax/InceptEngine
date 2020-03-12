#ifndef SKELETON_MESH_H
#define SKELETON_MESH_H

#include <iostream>
#include <string>
#include "Vertex.h"
#include <future>
class Renderer;
class Animation;
struct VertexBuffer;
struct IndexBuffer;
struct Texture;
struct UniformBuffer;
struct DataDescription;
class Skeleton;
struct Light;
struct Pipeline;
struct ShaderPath;
struct CubeMap;


class SkeletonMesh
{
public:
	SkeletonMesh(Renderer* renderer, const std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, ShaderPath shaderpath, std::string texturePath, Skeleton* skeleton, VkPrimitiveTopology topology);

	SkeletonMesh(Renderer* renderer, const std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, ShaderPath shaderpath, std::string cubemapPath[6], VkPrimitiveTopology topology);

	~SkeletonMesh();

	std::mutex m_boneTLock;

	static SkeletonMesh* loadSkeletonMesh(Renderer* renderer, const std::string& filepath, ShaderPath shaderpath, std::string texturePath, const std::string& rootBoneName, bool importAllMesh);
	
	void loadAnimation(std::vector<std::string> filenames, std::string rootBoneName);

	Animation* m_currentAnim = nullptr;

	VkBuffer* getMeshVerticesBuffer();

	VkBuffer getMeshIndicesBuffer();

	uint32_t getNumIndices();

	Skeleton* getSkeleton();

	

	VkDescriptorSet* getDescritorset(int i);

	Pipeline* getPipeline();

	void initializeUniformBuffer(Renderer* renderer, const std::vector<glm::mat4>& transformations, Light* light);

	void initializeLight(Renderer* renderer, Light* light);

	void updateUniformBuffer(Renderer* renderer, const std::vector<glm::mat4>& transformations, Light* light);

	bool m_animSignal;

	std::future<void> m_animationHandle;

	std::vector<Animation*> m_animations;
	bool m_motionControlledByAnim = false;

private:
	static void addBoneToVertex(std::vector<Vertex>& vertices, unsigned int boneID, unsigned int vertexID, float weights);

private:

	VertexBuffer* m_vertexBuffer = nullptr;

	uint32_t n_indices = 0;

	IndexBuffer* m_indexBuffer = nullptr;

	Texture* m_texture = nullptr;

	CubeMap* m_cubemap = nullptr;

	UniformBuffer* m_uBuffer = nullptr;

	DataDescription* m_dataDesc = nullptr;

	Pipeline* m_pipeline = nullptr;

	Skeleton* m_skeleton = nullptr;
};

void playAnimation(SkeletonMesh* mesh, int animIndex, std::vector<glm::mat4>* boneT, bool loop);



#endif