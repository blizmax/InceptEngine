
/*
#include "Mesh.h"
#include "Renderer.h"



Mesh::Mesh(Renderer* renderer, const std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	m_vertexBuffer = renderer->createVertexBuffer(vertices);

	m_indexBuffer = renderer->createIndexBuffer(indices);

	n_indices = static_cast<uint32_t>(indices.size());
}

Mesh::~Mesh()
{
	delete m_vertexBuffer;


	delete m_indexBuffer;

	delete m_material;

	delete m_dataDesc;

	delete m_pipeline;
}

VkBuffer* Mesh::getMeshVerticesBuffer()
{
	return &m_vertexBuffer->m_vertexBuffer;
}

VkBuffer Mesh::getMeshIndicesBuffer()
{
	return m_indexBuffer->m_indexBuffer;
}

uint32_t Mesh::getNumIndices()
{
	return n_indices;
}

VkDescriptorSet* Mesh::getDescritorset(int i)
{
	return &m_dataDesc->m_descriptorSet[i];
}

Pipeline* Mesh::getPipeline()
{
	return m_pipeline;
}*/
