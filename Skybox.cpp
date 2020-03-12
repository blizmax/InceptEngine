#include "Skybox.h"
#include "GameWorld.h"
#include "SkeletonMesh.h"
#include "Utils.h"

Skybox::Skybox(GameWorld* world, std::string texturePath[6])
	:Actor(glm::mat4(1.0), world)
{
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
	vertices.reserve(8);

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

	SkeletonMesh* mesh = new SkeletonMesh(m_world->m_renderer.get(), vertices, indices, path, texturePath, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
	setSkeletonMesh(mesh);

	m_skyboxBoneT = std::vector<glm::mat4>(2, glm::mat4(1.0));
	getSkeletonMesh()->updateUniformBuffer(m_world->m_renderer.get(), m_skyboxBoneT, m_world->m_light.get());
}

void Skybox::update()
{
	getSkeletonMesh()->updateUniformBuffer(m_world->m_renderer.get(), m_skyboxBoneT, m_world->m_light.get());
}

Skybox::~Skybox()
{

}
