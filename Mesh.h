/*
#ifndef MESH_H
#define MESH_H


#include <iostream>
#include <string>
#include "Vertex.h"

class Renderer;

struct VertexBuffer;
struct IndexBuffer;
struct Material;
struct DataDescription;
struct Pipeline;


class Mesh
{
public:
	Mesh(Renderer* renderer, const std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);

	~Mesh();

	Pipeline* getPipeline();

	VkBuffer* getMeshVerticesBuffer();

	VkBuffer getMeshIndicesBuffer();

	VkDescriptorSet* getDescritorset(int i);

	uint32_t getNumIndices();


protected:

	VertexBuffer* m_vertexBuffer = nullptr;

	uint32_t n_indices = 0;

	IndexBuffer* m_indexBuffer = nullptr;

	Material* m_material = nullptr;

	DataDescription* m_dataDesc = nullptr;

	Pipeline* m_pipeline = nullptr;

};

#endif // !MESH_H

*/