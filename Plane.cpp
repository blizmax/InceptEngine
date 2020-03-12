#include "Plane.h"
#include "SkeletonMesh.h"
#include "Utils.h"
#include "GameWorld.h"
class GameWorld;

//unit square with length 1
Plane::Plane(GameWorld* world, ShaderPath shaderpath, std::string texturePath)
	:Actor(glm::mat4(1.0), world)
{
	Vertex v1 = {};
	v1.position = glm::vec4(0.5, 0, 0.5, 1);
	v1.texCoord = glm::vec3(0, 0, 0);
	v1.vertexNormal = glm::vec3(0, 1, 0);
	Vertex v2 = {};
	v2.position = glm::vec4(0.5, 0, -0.5, 1);
	v2.texCoord = glm::vec3(0, 1, 0);
	v2.vertexNormal = glm::vec3(0, 1, 0);
	Vertex v3 = {};
	v3.position = glm::vec4(-0.5, 0, -0.5, 1);
	v3.texCoord = glm::vec3(1, 1, 0);
	v3.vertexNormal = glm::vec3(0, 1, 0);
	Vertex v4 = {};
	v4.position = glm::vec4(-0.5, 0, 0.5, 1);
	v4.texCoord = glm::vec3(1, 0, 0);
	v4.vertexNormal = glm::vec3(0, 1, 0);


	std::vector<Vertex> vertices = { v1, v2, v3, v4 };
	std::vector<uint32_t> indices = { 0,1,2,2,3,0 };
	SkeletonMesh* mesh = new SkeletonMesh(m_world->m_renderer.get(), vertices, indices, shaderpath, texturePath, nullptr, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	setSkeletonMesh(mesh);

	m_planeBoneT = std::vector<glm::mat4>(2, glm::mat4(1.0));
	m_planeBoneT[1] = getActorTransformation();
	getSkeletonMesh()->initializeUniformBuffer(m_world->m_renderer.get(), m_planeBoneT, m_world->m_light.get());

}


