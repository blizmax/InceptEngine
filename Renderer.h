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
#include "Utils.h"


#define GRAPHICS_QUEUE "graphicsQueue"
#define PRESENT_QUEUE "presentQueue"
const uint32_t n_buffers = 2;

class Renderer;

class Terrain;

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


enum class RenderObjectType
{
	SkeletonMesh,
	Skybox,
	Terrain
};


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


struct Pipeline
{
	Pipeline(Renderer* renderer);
	~Pipeline();
	Renderer* m_renderer;
	VkPipeline m_pipeline;
	VkPipelineLayout m_pipelineLayout;
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


struct CubeMap
{
	CubeMap(Renderer* renderer);
	~CubeMap();
	Renderer* m_renderer;

	VkImage m_cubeMapImage;
	VkDeviceMemory m_cubeMapImageMemory;
	VkImageView m_view;
	VkSampler m_sampler;
};


struct Material
{
	Material();
	~Material();
	Texture* m_texture = nullptr;
	CubeMap* m_cubeMap = nullptr;
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

class Skybox;

class Renderer
{

public:

	void setSkybox(Skybox* skybox);

	Renderer();

	~Renderer();

	GLFWwindow** getWindow();

	VkDescriptorSetLayout createSkyboxDescriptorSetLayout();

	std::array<VkDescriptorSet, n_buffers> createSkyboxDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorPool pool, const UniformBuffer& uBuffer, const CubeMap& cubemap);

	DataDescription* createSkyboxDataDescription(const UniformBuffer& uBuffer, const CubeMap& cubemap);

	void drawFrame();


	bool isChangingSize = false;

	MVP m_mvp = {};
	
	VkClearColorValue m_clearColor = {};
	
	VkClearValue m_clearValue = {};

	VertexBuffer* createVertexBuffer(const std::vector<Vertex>& vertices);

	IndexBuffer* createIndexBuffer(const std::vector<uint32_t>& indices);

	UniformBuffer* createUniformBuffer();

	DataDescription* createDataDescription(const UniformBuffer& uBuffer, const Texture& texture);

	Pipeline* createPipeline(ShaderPath shaderpath, DataDescription* dataDesc, VkPrimitiveTopology topology);

	CubeMap* createCubeMap(std::string texturePaths[6]);

	void initializeUniformBuffer(UniformBuffer& uBuffer, const std::vector<glm::mat4>& transformations, Light* light);

	void initializeLight(UniformBuffer& uBuffer, Light* light);

	void updateUniformBuffer(UniformBuffer& uBuffer, const std::vector<glm::mat4>& transformations, Light* light);

	void init();

	VkDevice getDevice();

	void spawnActor(Actor* actor);

	Texture* createTexture(std::string filePath);

	VkDescriptorPool createDescriptorPool();

	VkDescriptorSetLayout createDescriptorSetLayout();

	std::array<VkDescriptorSet, n_buffers> createDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorPool pool, const UniformBuffer& uBuffer, const Texture& texture);

	size_t getNumActors();
	
	void setTerrain(Terrain* terrain);
	
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
	
	

	void cleaupSwapChain();


	uint32_t findSuitableMemType(uint32_t filter, VkMemoryPropertyFlags properties);

	void createBuffer(  VkBuffer& buffer,
						VkBufferUsageFlags usageFlags,
						VkDeviceSize size,
						VkDeviceMemory& memory,
						VkMemoryPropertyFlags memProperties);
	
	void copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

	void recordCommandBuffer(uint32_t i);

	void createImage(VkImage& image, 
		VkDeviceMemory& imageMem, 
		uint32_t width, 
		uint32_t height, 
		uint32_t depth, 
		uint32_t arrayLayers, 
		uint32_t mipLevels, 
		VkImageCreateFlags flags, 
		VkImageType type, 
		VkFormat format, 
		VkImageTiling tiling, 
		VkImageUsageFlags usage, 
		VkMemoryPropertyFlags memoryProperties);


	VkCommandBuffer beginSingleTimeCommandBuffer();

	void endSingleTimeCommandBuffer(VkCommandBuffer& buffer);

	void transitImageLayout(VkImage& image,
		uint32_t numLayer,
		VkImageLayout oldLayout,
		VkImageLayout newLayout,
		VkAccessFlags srcAccessMask,
		VkAccessFlags dstAccessMask,
		VkImageAspectFlags aspectFlag,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask);


	VkImageView createImageView(VkImage& image, uint32_t numLayers, VkImageAspectFlags aspectMask, VkFormat viewFormat, VkImageViewType viewType);

	VkSampler createTextureSampler();

	void cleanup();

private:
	Skybox* m_skybox = nullptr;

	Terrain* m_terrain = nullptr;

	uint32_t m_nextRenderImgIdx = 0;

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

	//VkPipelineLayout m_pipelineLayout;

	VkRenderPass m_renderPass = nullptr;

	std::vector<VkFramebuffer> m_framebuffers;

	bool m_minimized = false;
	
	VkPhysicalDeviceMemoryProperties m_physicalDeviceMemProp;


	std::recursive_mutex m_actorLock;

	std::vector<Actor*> m_actors = {};

};



#endif