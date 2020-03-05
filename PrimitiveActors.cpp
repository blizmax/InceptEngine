#include "PrimitiveActors.h"
#include "SkeletonMesh.h"
#include "Renderer.h"

//unit square with length 1
Plane::Plane(Renderer* renderer, ShaderPath shaderpath, std::string texturePath, Light* light)
	:Actor(glm::mat4(1.0))
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
	SkeletonMesh* mesh = new SkeletonMesh(renderer, vertices, indices, shaderpath, texturePath, nullptr);
	mesh->updateUniformBuffer(renderer, getIdentityTransformationBuffer(), light);

	setSkeletonMesh(mesh);

}

std::vector<glm::mat4> getIdentityTransformationBuffer()
{
	std::vector<glm::mat4> tBuffer = {};
	tBuffer.resize(2);
	tBuffer[0] = glm::mat4(1.0f);
	tBuffer[1] = glm::mat4(1.0f);
	return tBuffer;
}
