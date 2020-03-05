#include "Skybox.h"
#include "Renderer.h"

#include "SkeletonMesh.h"

Skybox::Skybox(Renderer* renderer, std::string texturePath[6])
	:Actor(glm::mat4(1.0))
{
	m_cubeMap = nullptr;
	m_cubeMap = renderer->createCubeMap(texturePath);
	/*
	const glm::vec4 vertexPositions[8] =
	{
		glm::vec4(-0.5f, -0.5f, -0.5f, 1.0f),
		glm::vec4(0.5f, -0.5f, -0.5f, 1.0f),
		glm::vec4(0.5f, -0.5f, 0.5f, 1.0f),
		glm::vec4(-0.5f, -0.5f, 0.5f, 1.0f),
		glm::vec4(-0.5f, 0.5f, -0.5f, 1.0f),
		glm::vec4(0.5f, 0.5f, -0.5f, 1.0f),
		glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
		glm::vec4(-0.5f, 0.5f, 0.5f, 1.0f)
	};*/
	const glm::vec4 vertexPositions[8] =
	{
		glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f),
		glm::vec4(1.0f, -1.0f, -1.0f, 1.0f),
		glm::vec4(1.0f, -1.0f, 1.0f, 1.0f),
		glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f),
		glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f),
		glm::vec4(1.0f, 1.0f, -1.0f, 1.0f),
		glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
		glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f)
	};

	std::vector<Vertex> vertices;
	
	for (int i = 0; i < 8; i++)
	{
		Vertex v;
		v.position = vertexPositions[i];
		vertices.push_back(v);
	}



	std::vector<uint32_t> indices = { 0,1,5, 5,4,0, 2,3,7,7,6,2, 4,5,6, 6,7,4,  0,3,2, 2,1,0, 1,2,6, 6,5,1, 0,4,7, 7,3,0 };
	ShaderPath path =
	{
		"D:\\Inception\\Content\\Shaders\\spv\\skyboxVertex.spv",
		"D:\\Inception\\Content\\Shaders\\spv\\skyboxFrag.spv"
	};

	SkeletonMesh* mesh = new SkeletonMesh(renderer, vertices, indices, path, texturePath);
	setSkeletonMesh(mesh);
}

Skybox::~Skybox()
{
	delete m_cubeMap;
}
