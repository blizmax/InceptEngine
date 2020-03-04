#include "Vertex.h"


VkVertexInputBindingDescription Vertex::getVertexBindingDesc()
{
	VkVertexInputBindingDescription desc = {};
	desc.binding = 0;
	desc.stride = sizeof(Vertex);
	desc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return desc;

}

std::vector<VkVertexInputAttributeDescription> Vertex::getVertexAttriDesc()
{
	std::vector<VkVertexInputAttributeDescription> arr;
	arr.resize(6);

	arr[0].binding = 0;
	arr[0].location = 0;
	arr[0].offset = offsetof(Vertex, position);
	arr[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;


	arr[1].binding = 0;
	arr[1].location = 1;
	arr[1].offset = offsetof(Vertex, color);
	arr[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;

	arr[2].binding = 0;
	arr[2].location = 2;
	arr[2].offset = offsetof(Vertex, texCoord);
	arr[2].format = VK_FORMAT_R32G32B32_SFLOAT;

	arr[3].binding = 0;
	arr[3].location = 3;
	arr[3].offset = offsetof(Vertex, vertexNormal);
	arr[3].format = VK_FORMAT_R32G32B32_SFLOAT;

	arr[4].binding = 0;
	arr[4].location = 4;
	arr[4].offset = offsetof(Vertex, boneWeights);
	arr[4].format = VK_FORMAT_R32G32B32A32_SFLOAT;

	arr[5].binding = 0;
	arr[5].location = 5;
	arr[5].offset = offsetof(Vertex, affectedBonesID);
	arr[5].format = VK_FORMAT_R32G32B32A32_UINT;

	return arr;
}

