#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cassert>
#include <vector>
#include <iostream>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <fstream>
#define GLM_FORCE_RADIANS
#define GLM_FORECE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <array>
#include<glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/quaternion.hpp>
#include <chrono>

#define GRAPHICS_QUEUE "graphicsQueue"
#define PRESENT_QUEUE "presentQueue"

const int MAX_BONE_PER_VERTEX = 4;
const int MAX_BONE_PER_SKELETON = 200;

struct Camera
{

	glm::vec4 m_x = { 1,0,0,0 };
	glm::vec4 m_y = { 0,1,0,0 };
	glm::vec4 m_z = { 0,0,1,0 };
	glm::vec4 m_pos = { 0,0,500,1 };



	glm::mat4 getCameraFrame()
	{
		return glm::mat4(m_x, m_y, m_z, m_pos);
	}

	glm::mat4 getCameraMatrix()
	{
		return glm::inverse(getCameraFrame());
	}

	void rotateLocal(float degree, glm::vec3 axis)
	{
		glm::mat4  rotation = glm::rotate(glm::mat4(1.0), glm::radians(degree), axis);
		m_x = rotation * m_x;
		m_y = rotation * m_y;
		m_z = rotation * m_z;
	}

	void translate(float distance, glm::vec3 direction)
	{
		glm::mat4 translate = glm::translate(glm::mat4(1.0), distance * direction);
		m_pos = translate * m_pos;
		std::cout << glm::to_string(m_pos) << std::endl;
	}

};



struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamilyIndex;
	std::optional<uint32_t> presentFamilyIndex;

	bool isComplete()
	{
		return graphicsFamilyIndex.has_value() && presentFamilyIndex.has_value();
	}
};

struct Vertex
{
	glm::vec4 position = glm::vec4(0);
	glm::vec4 color = glm::vec4(0);
	glm::vec4 boneWeights = { 0.0,0.0,0.0,0.0 };
	glm::uvec4 affectedBonesID= { 0,0,0,0 };

	static VkVertexInputBindingDescription getVertexBindingDesc();
	static std::vector<VkVertexInputAttributeDescription> getVertexAttriDesc();
};

struct MVP
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};


class Renderer
{
public:
	std::vector<Vertex> m_vertices = {
		{{1.0, 1.0, -1.0, 1.0}, {0.0,0.0,0.0,1.0}},
		{{1.0, 1.0, 1.0, 1.0}, {0.0,0.0,0.0,1.0}},
		{{-1.0, 1.0, 1.0, 1.0}, {0.0,0.0,0.0,1.0}},
		{{-1.0, 1.0, -1.0, 1.0}, {0.0,0.0,0.0,1.0}},
		{{-1.0, -1.0, -1.0, 1.0}, {0.0,0.0,0.0,1.0}},
		{{-1.0, -1.0, 1.0, 1.0}, {0.0,0.0,0.0,1.0}},
		{{1.0, -1.0, 1.0, 1.0}, {0.0,0.0,0.0,1.0}},
		{{1.0, -1.0, -1.0, 1.0}, {0.0,0.0,0.0,1.0}}
	};



	std::vector<uint32_t> m_indices = { 0,1,2,1,2,3,
										2,3,5,3,4,5,
										0,3,4,0,4,7,
										1,2,5,5,1,6,
										4,5,6,6,4,7,
										0,7,6,6,0,1};


	Renderer();

	~Renderer();

	GLFWwindow** getWindow()
	{
		return &m_window;
	}


	
	void clearScreen();

	void drawTriangle(const std::vector<glm::mat4>& boneT);

	void resizeWindow(int width, int height);

	bool isChangingSize = false;
	MVP m_mvp = {};
	
	VkClearColorValue m_clearColor = {};
	
	VkClearValue m_clearValue = {};
	
	void setVertices(std::vector<Vertex> v, std::vector<uint32_t> idx)
	{
		m_vertices = v;
		m_indices = idx;
	}

	void init();
private:


	void createWindow();

	void createInstance();

	void createSurface();

	void createPhysicalDevice();

	bool checkPhysicalDeviceExtensionSupport();

	void getQueueFamilyIndices();

	void createLogicalDevice();
	
	void createSwapchain();
	
	void createImageView();

	void createRenderPass();

	void createFramebuffer();

	void createCommandPool();

	void createCommandBuffers();
	
	void recordCommandBuffers();
	
	void createSemaphore();

	std::vector<char> loadShaderBinary(const std::string& filePath);

	VkShaderModule loadShaderModule(const std::string& filePath);
	
	void createGraphicsPipeline();

	void cleaupSwapChain();

	void recreateSwapChain();

	void createBuffer(  VkBuffer& buffer,
						VkBufferUsageFlags usageFlags,
						VkDeviceSize size,
						VkDeviceMemory& memory,
						VkMemoryPropertyFlags memProperties);
	
	void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

	void createVertexBuffer(const std::vector<Vertex>& vertices);

	void createIndexBuffer(const std::vector<uint32_t>& indices);

	void createUniformBuffer();

	void createDescriptorSetLayout();

	void createDescriptorPool();

	void createDescriptorSets();

	void updateUniformBuffer(const std::vector<glm::mat4>& boneTransforms);

	
	void updateFrame();

	void cleanup();

	
	


private:
	const uint32_t n_buffers = 2;
	

	uint32_t m_currentRenderingImgIdx = 0;

	uint32_t m_windowWidth;
	uint32_t m_windowHeight;
	GLFWwindow* m_window;

	VkInstance m_instance;
	
	VkSurfaceKHR m_surface;

	VkPhysicalDevice m_physicalDevice;
	QueueFamilyIndices m_queueFamilyIndices;

	const std::vector<const char*> m_deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

	VkDevice m_device;

	VkFormat m_imageFormat;

	VkExtent2D m_swapchainExtent;
	VkSwapchainKHR m_swapchain;
	std::vector<VkImage> m_swapchainImages;
	std::vector<VkImageView> m_imageViews;


	VkCommandPool m_commandPool;

	std::unordered_map<std::string, VkQueue> m_queues;

	
	std::vector<VkSemaphore> m_graphicsFinishSemaphores;
	std::vector<VkFence> m_imageFinishRenderFence;
	VkFence m_acquireImageFence;


	std::vector<VkCommandBuffer> m_commandBuffers;

	VkPipelineLayout m_pipelineLayout;

	VkRenderPass m_renderPass = nullptr;

	std::vector<VkFramebuffer> m_framebuffers;

	VkPipeline m_graphicsPipeline;

	bool m_minimized = false;
	
	VkPhysicalDeviceMemoryProperties m_physicalDeviceMemProp;

	uint32_t findSuitableMemType(uint32_t filter, VkMemoryPropertyFlags properties);

	VkBuffer m_vertexBuffer;

	VkDeviceMemory m_vertexBufferMem;

	VkBuffer m_indexBuffer;

	VkDeviceMemory m_indexBufferMem;

	VkDescriptorSetLayout m_descriptorSetLayout;

	VkDescriptorPool m_descriptorSetPool;
	std::vector<VkDescriptorSet> m_descriptorSets;

	std::vector<VkBuffer> m_uniformBuffers;
	std::vector<VkDeviceMemory> m_uniformBufferMem;
};


