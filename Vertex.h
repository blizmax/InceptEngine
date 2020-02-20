#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>


#define GLM_FORCE_RADIANS
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


const int MAX_BONE_PER_VERTEX = 4;
const int MAX_BONE_PER_SKELETON = 200;

#ifndef VERTEX_H
#define VERTEX_H
struct Vertex
{
	glm::vec4 position = glm::vec4(0);
	glm::vec4 color = glm::vec4(0);
	glm::vec4 boneWeights = { 0.0,0.0,0.0,0.0 };
	glm::uvec4 affectedBonesID = { 0,0,0,0 };

	static VkVertexInputBindingDescription getVertexBindingDesc();
	static std::vector<VkVertexInputAttributeDescription> getVertexAttriDesc();
};
#endif