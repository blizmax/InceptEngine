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
#include <mutex>

#include "Math.h"
#include "Vertex.h"



#define GRAPHICS_QUEUE "graphicsQueue"
#define PRESENT_QUEUE "presentQueue"
const uint32_t n_buffers = 2;

class Actor;

struct Light;

struct MVP
{
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

class Renderer;

struct VertexBuffer
{
	VertexBuffer(Renderer* renderer);
	~VertexBuffer();

	Renderer* m_renderer;
	VkBuffer m_vertexBuffer;
	VkDeviceMemory m_vertexBufferMemory;
};

struct IndexBuffer
{
	IndexBuffer(Renderer* renderer);
	~IndexBuffer();
	Renderer* m_renderer;
	VkBuffer m_indexBuffer;
	VkDeviceMemory m_indexBufferMemory;
};

struct Texture
{
	Texture(Renderer* renderer);
	~Texture();
	Renderer* m_renderer;
	VkImage m_textureImage;
	VkDeviceMemory m_textureImageMemory;
	VkImageView m_view;
	VkSampler m_sampler;
};


//the transformation buffer, by design, is a vulkan uniform buffer of array of 200 glm::mat4
//the first mat4 is always identity, for the usage of static mesh
//the second mat4 is the model transformation matrix
//the last 198 matrix are the bone transformation matrices

struct UniformBuffer
{
	UniformBuffer(Renderer* renderer);
	~UniformBuffer();
	Renderer* m_renderer;
	std::array<VkBuffer, n_buffers> m_boneTransform;
	std::array<VkDeviceMemory, n_buffers> m_boneTransformMemory;
	std::array<VkBuffer, n_buffers> m_light;
	std::array<VkDeviceMemory, n_buffers> m_lightMemory;

};

//this struct wrap up the vkdescriptor creation
//which tell the shader the layout of the data in the 
//uniform buffer object
struct DataDescription
{
	DataDescription(Renderer* renderer);
	~DataDescription();
	Renderer* m_renderer;
	VkDescriptorPool m_pool;
	VkDescriptorSetLayout m_layout;
	std::array<VkDescriptorSet, 2> m_descriptorSet;
};


class Renderer
{

public:



	Renderer();

	~Renderer();

	GLFWwindow** getWindow();


	void drawFrame();

	void resizeWindow(int width, int height);

	bool isChangingSize = false;

	MVP m_mvp = {};
	
	VkClearColorValue m_clearColor = {};
	
	VkClearValue m_clearValue = {};

	

	VertexBuffer* createVertexBuffer(const std::vector<Vertex>& vertices);

	IndexBuffer* createIndexBuffer(const std::vector<uint32_t>& indices);

	UniformBuffer* createUniformBuffer();

	DataDescription* createDataDescription(const UniformBuffer& uBuffer, const Texture& texture);

	void initializeUniformBuffer(UniformBuffer& uBuffer, const std::vector<glm::mat4>& transformations, Light* light);

	void updateUniformBuffer(UniformBuffer& uBuffer, const std::vector<glm::mat4>& transformations, Light* light);

	void init();

	VkDevice getDevice();

	void spawnActor(Actor* actor);

	Texture* createTexture(std::string filePath);

	VkDescriptorPool createDescriptorPool();

	VkDescriptorSetLayout createDescriptorSetLayout();

	std::array<VkDescriptorSet, n_buffers> createDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorPool pool, const UniformBuffer& uBuffer, const Texture& texture);

	size_t getNumActors();
	
	
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
	
	void createSemaphoreAndFence();

	std::vector<char> loadShaderBinary(const std::string& filePath);

	VkShaderModule loadShaderModule(const std::string& filePath);
	
	void createGraphicsPipeline();

	void cleaupSwapChain();

	void recreateSwapChain();

	uint32_t findSuitableMemType(uint32_t filter, VkMemoryPropertyFlags properties);

	void createBuffer(  VkBuffer& buffer,
						VkBufferUsageFlags usageFlags,
						VkDeviceSize size,
						VkDeviceMemory& memory,
						VkMemoryPropertyFlags memProperties);
	
	void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

	void recordCommandBuffer(uint32_t i);

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

	VkSampler createTextureSampler();

	void cleanup();

private:
	
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

	//VkDescriptorPool m_descriptorSetPool;

	std::recursive_mutex m_actorLock;

	std::vector<Actor*> m_actors = {};

};



#endif