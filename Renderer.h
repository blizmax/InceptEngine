#ifndef RENDERER_H
#define RENDERER_H

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
#include <array>
#include <chrono>

#include "Math.h"
#include "Vertex.h"



#define GRAPHICS_QUEUE "graphicsQueue"
#define PRESENT_QUEUE "presentQueue"



struct MVP
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
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




class Renderer
{
public:
	std::vector<Vertex> m_vertices = {};

	std::vector<uint32_t> m_indices = {};



	Renderer();

	~Renderer();

	GLFWwindow** getWindow()
	{
		return &m_window;
	}


	
	void clearScreen();

	void drawFrame();

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

	void addVertices(const std::vector<Vertex>& v, const std::vector<uint32_t>& idx);

	void updateUniformBuffer(const std::vector<glm::mat4>& boneTransforms);

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
	
	void createImageViews();

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


	void recordCommandBuffer(uint32_t i);

	
	void createTextureImage();
	
	void updateFrame();

	void createImage(VkImage& image, VkDeviceMemory& imageMem, uint32_t width, uint32_t height, uint32_t depth, VkImageType type, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties);

	VkCommandBuffer beginSingleTimeCommandBuffer();

	void endSingleTimeCommandBuffer(VkCommandBuffer& buffer);

	void transitImageLayout(VkImage& image,
							VkImageLayout oldLayout,
							VkImageLayout newLayout,
							VkAccessFlags srcAccessMask,
							VkAccessFlags dstAccessMask,
							VkImageAspectFlags aspectFlag,
							VkPipelineStageFlags srcStageMask,
							VkPipelineStageFlags dstStageMask);


	VkImageView createImageView(VkImage& image, VkImageAspectFlags aspectMask, VkFormat viewFormat, VkImageViewType viewType);

	void createTextureSampler();

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

	VkImage m_textureIamge;
	VkDeviceMemory m_textureImageMem;
	VkImageView m_textureImageView;

	VkSampler m_textureSampler;
};


#endif