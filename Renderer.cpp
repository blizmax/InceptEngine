#include "Renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "Actor.h"
#include "SkeletonMesh.h"
#include "Light.h"
#include "Terrain.h"
#include "Skybox.h"

DataDescription* Renderer::createDataDescription(const UniformBuffer& uBuffer, const Texture& texture)
{
	DataDescription* desc = new DataDescription(this);
	desc->m_pool = createDescriptorPool();
	desc->m_layout = createDescriptorSetLayout();
	desc->m_descriptorSet = createDescriptorSet(desc->m_layout, desc->m_pool, uBuffer, texture);
	return desc;
}

DataDescription* Renderer::createSkyboxDataDescription(const UniformBuffer& uBuffer, const CubeMap& cubemap)
{
	DataDescription* desc = new DataDescription(this);
	desc->m_pool = createDescriptorPool();
	desc->m_layout = createSkyboxDescriptorSetLayout();
	desc->m_descriptorSet = createSkyboxDescriptorSet(desc->m_layout, desc->m_pool, uBuffer, cubemap);
	return desc;
}

void Renderer::setSkybox(Skybox* skybox)
{
	if (m_skybox != nullptr) delete m_skybox;
	m_skybox = skybox;
}

Renderer::Renderer()
{
	m_clearColor.float32[0] = 0.5f;
	m_clearColor.float32[1] = 0.0f;
	m_clearColor.float32[2] = 0.0f;
	m_clearColor.float32[3] = 1.0f;
	m_clearValue.color = m_clearColor;


	init();
}

std::vector<char> Renderer::loadShaderBinary(const std::string& filePath)
{
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "Cannot open file " << filePath << std::endl;
		throw std::runtime_error("");
	}
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> binary(fileSize);
	file.seekg(0);
	file.read(binary.data(), fileSize);
	file.close();
	return binary;

}




void Renderer::drawFrame()
{
	if (isChangingSize || m_minimized)
	{
		return;
	}
	
	

	if (vkAcquireNextImageKHR(m_device, m_swapchain, UINT64_MAX, VK_NULL_HANDLE, m_acquireImageFence, &m_currentRenderingImgIdx) != VK_SUCCESS)
	{
		std::cerr << "Fail to get next available image from the swap chain" << std::endl;
		throw std::runtime_error("");
	}
	
	vkWaitForFences(m_device, 1, &m_acquireImageFence, VK_TRUE, UINT64_MAX);
	
	vkResetFences(m_device, 1, &m_acquireImageFence);

	vkWaitForFences(m_device, 1, &m_imageFinishRenderFence[m_currentRenderingImgIdx], VK_TRUE, UINT64_MAX);

	vkResetFences(m_device, 1, &m_imageFinishRenderFence[m_currentRenderingImgIdx]);

	recordCommandBuffer(m_currentRenderingImgIdx);

	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_commandBuffers[m_currentRenderingImgIdx];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_graphicsFinishSemaphores[m_currentRenderingImgIdx];
	vkQueueSubmit(m_queues.at(GRAPHICS_QUEUE), 1, &submitInfo, m_imageFinishRenderFence[m_currentRenderingImgIdx]);

	VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_graphicsFinishSemaphores[m_currentRenderingImgIdx];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_swapchain;
	presentInfo.pImageIndices = &m_currentRenderingImgIdx;
	vkQueuePresentKHR(m_queues.at(PRESENT_QUEUE), &presentInfo);

}



void Renderer::init()
{
	createWindow();

	createInstance();

	createSurface();
	
	createPhysicalDevice();

	createLogicalDevice();

	createSwapchain();

	createImageViews();

	createCommandPool();

	createRenderPass();

	createFramebuffer();

	createSemaphoreAndFence();

	createCommandBuffers();

}

VkDevice Renderer::getDevice()
{
	return m_device;
}

void Renderer::spawnActor(Actor* actor)
{
	m_actorLock.lock();
	m_actors.push_back(actor);
	m_actorLock.unlock();
}

Texture* Renderer::createTexture(std::string filePath)
{
	Texture* texture = new Texture(this);

	int width, height, nChannel;
	stbi_uc* pixels = stbi_load(filePath.c_str(), &width, &height, &nChannel, STBI_rgb_alpha);
	if (pixels == nullptr)
	{
		std::cerr << "Fail to load texture!" << std::endl;
		throw std::runtime_error("");
	}

	VkDeviceSize imageSize = width * height * 4;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMem;
	createBuffer(stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, imageSize, stagingBufferMem, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	void* data;
	vkMapMemory(m_device, stagingBufferMem, 0, imageSize, 0, &data);
	memcpy(data, reinterpret_cast<const void*>(pixels), static_cast<size_t>(imageSize));
	vkUnmapMemory(m_device, stagingBufferMem);
	stbi_image_free(pixels);

	createImage(texture->m_textureImage, 
		texture->m_textureImageMemory, 
		static_cast<uint32_t>(width), 
		static_cast<uint32_t>(height),
		1, 
		1,
		1,
		0,
		VK_IMAGE_TYPE_2D, 
		VK_FORMAT_R8G8B8A8_UNORM, 
		VK_IMAGE_TILING_OPTIMAL, 
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, 
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	transitImageLayout(texture->m_textureImage, 1, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkBufferImageCopy region = {};
	region.bufferImageHeight = 0;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;

	region.imageExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };
	region.imageOffset = { 0,0,0 };
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;
	region.imageSubresource.mipLevel = 0;

	auto commandBuffer = beginSingleTimeCommandBuffer();
	vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, texture->m_textureImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	endSingleTimeCommandBuffer(commandBuffer);

	transitImageLayout(texture->m_textureImage, 1, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_ASPECT_COLOR_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMem, nullptr);

	texture->m_view = createImageView(texture->m_textureImage, 1, VK_IMAGE_ASPECT_COLOR_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_VIEW_TYPE_2D);

	texture->m_sampler = createTextureSampler();
	
	return texture;
}

void Renderer::createWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	m_windowWidth = 1600;
	m_windowHeight = 900;
	m_window = glfwCreateWindow(m_windowWidth, m_windowHeight, "Inception Engine", nullptr, nullptr);

	

	m_swapchainExtent.width = m_windowWidth;
	m_swapchainExtent.height = m_windowHeight;
}

void Renderer::createInstance()
{
	VkInstance instance = 0;
	VkInstanceCreateInfo createInfo = { VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO };

	uint32_t enableExtensionCount = 0;
	const char** enableExtensionNames = glfwGetRequiredInstanceExtensions(&enableExtensionCount);


	const int enableLayerCount = 1;
	const char* enableLayerNames[enableLayerCount] = { "VK_LAYER_KHRONOS_validation" };
	VkApplicationInfo appInfo = { VK_STRUCTURE_TYPE_APPLICATION_INFO };
	const char* appName = "Inception";
	appInfo.pApplicationName = appName;
	appInfo.apiVersion = VK_API_VERSION_1_2;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = enableExtensionCount;
	createInfo.ppEnabledExtensionNames = enableExtensionNames;
	createInfo.enabledLayerCount = enableLayerCount;
	createInfo.ppEnabledLayerNames = enableLayerNames;
	if ( vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS )
	{
		std::cerr << "Fail to create vulkan instance!" << std::endl;
		throw std::runtime_error("");
	}

}

void Renderer::createPhysicalDevice()
{
	uint32_t physicalDeviceCount = 0;
	std::vector<VkPhysicalDevice> physcialDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, nullptr);
	physcialDevices.resize(physicalDeviceCount);

	vkEnumeratePhysicalDevices(m_instance, &physicalDeviceCount, physcialDevices.data());

	for (const auto& device : physcialDevices)
	{
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		{
			std::cout << "Select discrete card: " << deviceProperties.deviceName << std::endl;
			m_physicalDevice = device;
			vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_physicalDeviceMemProp);
			return;
		}

	}

	std::cout << "Select integrate card" << std::endl;
	m_physicalDevice = physcialDevices[0];

	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_physicalDeviceMemProp);
}

bool Renderer::checkPhysicalDeviceExtensionSupport()
{
	uint32_t extensionCount;
	std::vector<VkExtensionProperties> supportedExtensions;
	vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, nullptr);
	supportedExtensions.resize(extensionCount);
	vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extensionCount, supportedExtensions.data());
	std::set<std::string> extensionsToCheck(m_deviceExtensions.begin(), m_deviceExtensions.end());
	for (const auto& extension : supportedExtensions)
	{
		extensionsToCheck.erase(extension.extensionName);
	}
	return extensionsToCheck.empty();
}

void Renderer::getQueueFamilyIndices()
{
	QueueFamilyIndices indices;
	uint32_t count = 0;
	std::vector<VkQueueFamilyProperties> properties(count);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &count, nullptr);
	properties.resize(count);
	std::cout << "This graphics card supports " << count << " queue families." << std::endl;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &count, properties.data());
	
	
	uint32_t i = 0;
	for (const auto& deviceProperty : properties)
	{
		if (deviceProperty.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamilyIndex = i;
			break;
		}
		i++;
	}

	i = 0;
	VkBool32 support = false;
	for (const auto& deviceProperty : properties)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &support);
		if (support)
		{
			indices.presentFamilyIndex = i;
			break;
		}
		i++;
	}
	m_queueFamilyIndices = indices;
	m_queueFamilyIndices = indices;
}

void Renderer::createLogicalDevice()
{
	if (!checkPhysicalDeviceExtensionSupport())
	{
		std::cerr << "Your video card does not support swapchain extension!" << std::endl;
		throw std::runtime_error("");
	}

	getQueueFamilyIndices();

	if (!m_queueFamilyIndices.isComplete())
	{
		std::cerr << "Your video card does not support both rendering and presenting!" << std::endl;
		throw std::runtime_error("");
	}

	VkDeviceQueueCreateInfo deviceQueueCreateInfo = { VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO };
	deviceQueueCreateInfo.queueCount = 1;
	deviceQueueCreateInfo.queueFamilyIndex = m_queueFamilyIndices.graphicsFamilyIndex.value();
	float queuePriorty = 1.0f;
	deviceQueueCreateInfo.pQueuePriorities = &queuePriorty;
	VkDevice device = 0;

	VkPhysicalDeviceFeatures enabledFeatures = {};
	enabledFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	createInfo.queueCreateInfoCount = 1;
	createInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
	createInfo.pEnabledFeatures = &enabledFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();
	
	
	if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
	{
		std::cerr << "Fail to create logical device!" << std::endl;
		throw std::runtime_error("");
	}
	
	VkQueue graphicsQueue = nullptr;
	vkGetDeviceQueue(m_device, m_queueFamilyIndices.graphicsFamilyIndex.value(), 0, &graphicsQueue);
	if (graphicsQueue == nullptr)
	{
		std::cerr << "Fail to get graphics queue!" << std::endl;
		throw std::runtime_error("");
	}
	m_queues.insert(std::pair(GRAPHICS_QUEUE, graphicsQueue));
	
	VkQueue presentQueue = nullptr;
	vkGetDeviceQueue(m_device, m_queueFamilyIndices.presentFamilyIndex.value(), 0, &presentQueue);
	if (presentQueue == nullptr)
	{
		std::cerr << "Fail to get graphics queue!" << std::endl;
		throw std::runtime_error("");
	}
	m_queues.insert(std::pair(PRESENT_QUEUE, presentQueue));
}

void Renderer::createSurface()
{
	if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface))
	{
		std::cerr << "Fail to create vulkan surface!" << std::endl;
		throw std::runtime_error("");
	}
}

void Renderer::createSwapchain()
{
	m_imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
	VkSwapchainCreateInfoKHR createInfo = { VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR };
	createInfo.surface = m_surface;
	createInfo.minImageCount = n_buffers;
	createInfo.imageFormat = m_imageFormat; //SHORTCUT!!
	createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	m_swapchainExtent = { m_windowWidth, m_windowHeight };
	m_mvp.proj = glm::perspective(glm::radians(45.0f), m_swapchainExtent.width / (float)m_swapchainExtent.height, 0.1f, 1000000.0f);
	createInfo.imageExtent = m_swapchainExtent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	if (m_queueFamilyIndices.graphicsFamilyIndex.value() == m_queueFamilyIndices.presentFamilyIndex.value())
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	else
	{
		std::cout << "The graphics queue and present queue are different" << std::endl;
		std::cout << "The index of graphcis queue is " << m_queueFamilyIndices.graphicsFamilyIndex.value() << std::endl;
		std::cout << "The index of present queue is " << m_queueFamilyIndices.presentFamilyIndex.value() << std::endl;
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;  
		const uint32_t indices[] = { m_queueFamilyIndices.graphicsFamilyIndex.value(), m_queueFamilyIndices.presentFamilyIndex.value() };
		createInfo.pQueueFamilyIndices = indices; 

	}
	
	createInfo.presentMode = VK_PRESENT_MODE_MAILBOX_KHR;
	createInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	createInfo.clipped = VK_TRUE;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	if (vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) != VK_SUCCESS)
	{
		std::cerr << "fail to create swapchain! Error code is: " << std::endl;
		throw std::runtime_error("");
	}

	uint32_t swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchainImageCount, nullptr);
	m_swapchainImages.resize(swapchainImageCount);
	if (vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchainImageCount, m_swapchainImages.data()) != VK_SUCCESS)
	{
		std::cerr << "Fail to get swapchin images!" << std::endl;
		throw std::runtime_error("");
	}	
}

void Renderer::createImageViews()
{
	m_imageViews.resize(m_swapchainImages.size());
	for (size_t i = 0; i < m_imageViews.size(); i++)
	{
		VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		createInfo.image = m_swapchainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = m_imageFormat;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.layerCount = 1;
		createInfo.subresourceRange.levelCount = 1;

		if (vkCreateImageView(m_device, &createInfo, nullptr, &m_imageViews[i]) != VK_SUCCESS)
		{
			std::cerr << "Fail to create " << i << " image view!" << std::endl;
			throw std::runtime_error("");
		}
	}
}

void Renderer::createFramebuffer()
{
	m_framebuffers.resize(m_imageViews.size());
	for (uint32_t i = 0; i < m_framebuffers.size(); i++)
	{
		VkFramebufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO };
		createInfo.width = m_swapchainExtent.width;
		createInfo.height = m_swapchainExtent.height;
		createInfo.layers = 1;
		createInfo.attachmentCount = 1;
		createInfo.pAttachments = &m_imageViews[i];
		assert(m_renderPass);
		createInfo.renderPass = m_renderPass;
		if (vkCreateFramebuffer(m_device, &createInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
		{
			std::cerr << "Fail to create framebuffer " << i << std::endl;
			throw std::runtime_error("");
		}
	}
}

void Renderer::createCommandPool()
{
	VkCommandPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	createInfo.queueFamilyIndex = m_queueFamilyIndices.graphicsFamilyIndex.value();
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if (vkCreateCommandPool(m_device, &createInfo, nullptr, &m_commandPool) != VK_SUCCESS)
	{
		std::cerr << "Fail to create command pool!" << std::endl;
		throw std::runtime_error("");
	}
}


void Renderer::createCommandBuffers()
{
	
	m_commandBuffers.resize(m_swapchainImages.size());
	VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocateInfo.commandPool = m_commandPool;
	allocateInfo.commandBufferCount = (uint32_t)m_swapchainImages.size();
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	if (vkAllocateCommandBuffers(m_device, &allocateInfo, m_commandBuffers.data()) != VK_SUCCESS)
	{
		std::cerr << "Fail to allocate command buffers!" << std::endl;
		throw std::runtime_error("");
	}

	

}



void Renderer::recordCommandBuffer(uint32_t i)
{
	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo);


	VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
	renderPassBeginInfo.renderPass = m_renderPass;
	renderPassBeginInfo.renderArea.offset = { 0,0 };
	renderPassBeginInfo.renderArea.extent = m_swapchainExtent;
	renderPassBeginInfo.framebuffer = m_framebuffers[i];
	renderPassBeginInfo.clearValueCount = 1;
	renderPassBeginInfo.pClearValues = &m_clearValue;

	vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	

	VkDeviceSize offset[] = { 0 };

	for (auto actor : m_actors)
	{
		vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, actor->getSkeletonMesh()->getPipeline()->m_pipeline);

		vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, actor->getSkeletonMesh()->getMeshVerticesBuffer(), offset);

		vkCmdBindIndexBuffer(m_commandBuffers[i], actor->getSkeletonMesh()->getMeshIndicesBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdPushConstants(m_commandBuffers[i], actor->getSkeletonMesh()->getPipeline()->m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MVP), (void*)&m_mvp);

		vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, actor->getSkeletonMesh()->getPipeline()->m_pipelineLayout, 0, 1, actor->getSkeletonMesh()->getDescritorset(i), 0, nullptr);

		vkCmdDrawIndexed(m_commandBuffers[i], actor->getSkeletonMesh()->getNumIndices(), 1, 0, 0, 0);
	}


	vkCmdEndRenderPass(m_commandBuffers[i]);
	vkEndCommandBuffer(m_commandBuffers[i]);

}

void Renderer::createSemaphoreAndFence()
{
	
	VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	VkFenceCreateInfo fenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	m_graphicsFinishSemaphores.resize((size_t)n_buffers);
	m_imageFinishRenderFence.resize((size_t)n_buffers);

	for (size_t i = 0; i < n_buffers; i++)
	{
		vkCreateSemaphore(m_device, &createInfo, nullptr, &m_graphicsFinishSemaphores[i]);
		vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_imageFinishRenderFence[i]);
	}

	fenceCreateInfo.flags = 0;
	vkCreateFence(m_device, &fenceCreateInfo, nullptr, &m_acquireImageFence);
}

VkShaderModule Renderer::loadShaderModule(const std::string& filePath)
{
	auto code = loadShaderBinary(filePath);
	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo createInfo = { VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	createInfo.codeSize = (uint32_t)code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	vkCreateShaderModule(m_device, &createInfo, nullptr, &shaderModule);
	return shaderModule;

}

Pipeline* Renderer::createPipeline(ShaderPath shaderpath, DataDescription* dataDesc, VkPrimitiveTopology topology)
{
	
	Pipeline* pipeline = new Pipeline(this);

	
	VkShaderModule vertexShader = loadShaderModule(shaderpath.vertexShaderPath);
	VkShaderModule fragmentShader = loadShaderModule(shaderpath.fragmentShaderPath);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertexShader;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = { VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragmentShader;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStageInfos[] = { vertShaderStageInfo, fragShaderStageInfo };


	//fix functions
	VkPipelineVertexInputStateCreateInfo vertInputStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	auto vertexAttriDesc = Vertex::getVertexAttriDesc();
	vertInputStateCreateInfo.vertexAttributeDescriptionCount = (uint32_t)vertexAttriDesc.size();
	vertInputStateCreateInfo.pVertexAttributeDescriptions = vertexAttriDesc.data();
	vertInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	vertInputStateCreateInfo.pVertexBindingDescriptions = &Vertex::getVertexBindingDesc();


	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssemblyStateCreateInfo.topology = topology;
	inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.width = (float)m_windowWidth;
	viewport.height = -(float)m_windowHeight;
	viewport.x = 0.0f;
	viewport.y = (float)m_windowHeight;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0,0 };
	scissor.extent = m_swapchainExtent;

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationStateCreateInfo.lineWidth = 1.0f;
	//two-sided or single-sided of the material
	rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	
	VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
	colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachmentState.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	colorBlendStateCreateInfo.attachmentCount = 1;
	colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
	colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;


	//pipeline layout
	VkPushConstantRange pushConstRange = {};
	pushConstRange.offset = 0;
	pushConstRange.size = sizeof(MVP);
	pushConstRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;


	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
	pipelineLayoutCreateInfo.pushConstantRangeCount = 1;
	pipelineLayoutCreateInfo.pPushConstantRanges = &pushConstRange;
	pipelineLayoutCreateInfo.setLayoutCount = 1;
	pipelineLayoutCreateInfo.pSetLayouts = &dataDesc->m_layout;
	if (vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, &pipeline->m_pipelineLayout) != VK_SUCCESS)
	{
		std::cerr << "Fail to create pipeline layout!" << std::endl;
		throw std::runtime_error("");
	}


	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = { VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	graphicsPipelineCreateInfo.stageCount = 2;
	graphicsPipelineCreateInfo.pStages = shaderStageInfos;
	graphicsPipelineCreateInfo.pVertexInputState = &vertInputStateCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.layout = pipeline->m_pipelineLayout;
	graphicsPipelineCreateInfo.renderPass = m_renderPass;
	graphicsPipelineCreateInfo.subpass = 0; //index of the subpass that this graphics pipeline will use
	
	if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &pipeline->m_pipeline) != VK_SUCCESS)
	{
		std::cerr << "Fail to create graphics pipeline!" << std::endl;
		throw std::runtime_error("");
	}
	
	

	vkDestroyShaderModule(m_device, vertexShader, nullptr);
	vkDestroyShaderModule(m_device, fragmentShader, nullptr);

	return pipeline;

}

void Renderer::cleaupSwapChain()
{
	//framebuffer
	for (size_t i = 0; i < n_buffers; i++)
	{
		vkDestroyFramebuffer(m_device, m_framebuffers[i], nullptr);
		vkFreeCommandBuffers(m_device, m_commandPool, 1, &m_commandBuffers[i]);
		vkDestroyImageView(m_device, m_imageViews[i], nullptr);
		vkDestroySemaphore(m_device, m_graphicsFinishSemaphores[i], nullptr);
		vkDestroyFence(m_device, m_imageFinishRenderFence[i], nullptr);
	}

	vkDestroyFence(m_device, m_acquireImageFence, nullptr);



	//renderpass
	vkDestroyRenderPass(m_device, m_renderPass, nullptr);

	//swapchian
	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

}



void Renderer::createBuffer(VkBuffer& buffer, 
							VkBufferUsageFlags usageFlags, 
							VkDeviceSize size, 
							VkDeviceMemory& memory,
							VkMemoryPropertyFlags memProperties)
{
	VkBufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	createInfo.usage = usageFlags;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.size = size;

	if (vkCreateBuffer(m_device, &createInfo, nullptr, &buffer) != VK_SUCCESS)
	{
		std::cerr << "Fail to create vertex buffer!" << std::endl;
		throw std::runtime_error("");
	}


	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(m_device, buffer, &memReq);
	VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = findSuitableMemType(memReq.memoryTypeBits, memProperties);

	if (vkAllocateMemory(m_device, &allocInfo, nullptr, &memory) != VK_SUCCESS)
	{
		std::cerr << "Fail to allocate memory for vertex buffer!" << std::endl;
		throw std::runtime_error("");
	}

	vkBindBufferMemory(m_device, buffer, memory, 0);
}

void Renderer::copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
{
	VkCommandBuffer copyCmdBuffer;
	VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.commandBufferCount = 1;
	allocInfo.commandPool = m_commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	if (vkAllocateCommandBuffers(m_device, &allocInfo, &copyCmdBuffer) != VK_SUCCESS)
	{
		std::cerr << "Fail to allocate copy command buffer!" << std::endl;
		throw std::runtime_error("");
	}
	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	
	if (vkBeginCommandBuffer(copyCmdBuffer, &beginInfo) != VK_SUCCESS)
	{
		std::cerr << "Fail to begin copy command buffer!" << std::endl;
		throw std::runtime_error("");
	}

	VkBufferCopy copyInfo;
	copyInfo.size = size;
	copyInfo.srcOffset = 0;
	copyInfo.dstOffset = 0;

	vkCmdCopyBuffer(copyCmdBuffer, src, dst, 1, &copyInfo);

	if (vkEndCommandBuffer(copyCmdBuffer) != VK_SUCCESS)
	{
		std::cerr << "Fail to end copy command buffer!" << std::endl;
		throw std::runtime_error("");
	}

	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCmdBuffer;
	vkQueueSubmit(m_queues.at(GRAPHICS_QUEUE), 1, &submitInfo, VK_NULL_HANDLE);
	vkDeviceWaitIdle(m_device);
	vkFreeCommandBuffers(m_device, m_commandPool, 1, &copyCmdBuffer);
	
}

VertexBuffer* Renderer::createVertexBuffer(const std::vector<Vertex>& vertices)
{
	VertexBuffer* vertexBuffer = new VertexBuffer(this);

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMem;
	VkDeviceSize size = sizeof(Vertex) * vertices.size();

	createBuffer(stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, stagingBufferMem, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data = nullptr;
	vkMapMemory(m_device, stagingBufferMem, 0, size, 0, &data);

	memcpy(data, vertices.data(), (size_t)size);

	vkUnmapMemory(m_device, stagingBufferMem);

	createBuffer(vertexBuffer->m_vertexBuffer, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, size, vertexBuffer->m_vertexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	copyBuffer(stagingBuffer, vertexBuffer->m_vertexBuffer, size);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);

	vkFreeMemory(m_device, stagingBufferMem, nullptr);
	
	return vertexBuffer;
	
}

IndexBuffer* Renderer::createIndexBuffer(const std::vector<uint32_t>& indices)
{
	IndexBuffer* indexBuffer = new IndexBuffer(this);

	VkDeviceSize size = indices.size() * sizeof(uint32_t);
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMem;
	createBuffer(stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, stagingBufferMem, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(m_device, stagingBufferMem, 0, size, 0, &data);
	memcpy(data, indices.data(), size);
	vkUnmapMemory(m_device, stagingBufferMem);

	createBuffer(indexBuffer->m_indexBuffer, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, size, indexBuffer->m_indexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	copyBuffer(stagingBuffer, indexBuffer->m_indexBuffer, size);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMem, nullptr);

	return indexBuffer;

}




void Renderer::createImage(VkImage& image, VkDeviceMemory& imageMem, uint32_t width, uint32_t height, uint32_t depth, uint32_t arrayLayers, uint32_t mipLevels, VkImageCreateFlags flags, VkImageType type, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags memoryProperties)
{
	VkImageCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	createInfo.flags = flags;
	createInfo.arrayLayers = arrayLayers;
	createInfo.extent.width = width;
	createInfo.extent.height = height;
	createInfo.extent.depth = depth;
	createInfo.format = format;
	createInfo.imageType = type;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createInfo.mipLevels = mipLevels;
	createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.tiling = tiling;
	createInfo.usage = usage;

	if (vkCreateImage(m_device, &createInfo, nullptr, &image) != VK_SUCCESS)
	{
		std::cerr << "Fail to create image!" << std::endl;
		throw std::runtime_error("");
	}

	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(m_device, image, &memReq);
	auto size = memReq.size;

	VkMemoryAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocInfo.allocationSize = size;
	allocInfo.memoryTypeIndex = memoryProperties;

	if (vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMem) != VK_SUCCESS)
	{
		std::cerr << "Fail to allocate image memory!" << std::endl;
		throw std::runtime_error("");
	}
	vkBindImageMemory(m_device, image, imageMem, 0);

}

VkCommandBuffer Renderer::beginSingleTimeCommandBuffer()
{
	VkCommandBuffer commandBuffer;
	VkCommandBufferAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocateInfo.commandPool = m_commandPool;
	allocateInfo.commandBufferCount = 1;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	vkAllocateCommandBuffers(m_device, &allocateInfo, &commandBuffer);
	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;

}

void Renderer::endSingleTimeCommandBuffer(VkCommandBuffer& buffer)
{
	vkEndCommandBuffer(buffer);

	VkFence fence;
	VkFenceCreateInfo fenceCreateInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
	vkCreateFence(m_device, &fenceCreateInfo, nullptr, &fence);

	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &buffer;
	vkQueueSubmit(m_queues.at(GRAPHICS_QUEUE), 1, &submitInfo, fence);

	vkWaitForFences(m_device, 1, &fence, VK_TRUE, UINT64_MAX);
	vkFreeCommandBuffers(m_device, m_commandPool, 1, &buffer);
	vkDestroyFence(m_device, fence, nullptr);

}

void Renderer::transitImageLayout(VkImage& image,
	uint32_t numLayers,
	VkImageLayout oldLayout, 
	VkImageLayout newLayout, 
	VkAccessFlags srcAccessMask, 
	VkAccessFlags dstAccessMask, 
	VkImageAspectFlags aspectFlag,
	VkPipelineStageFlags srcStageMask,
	VkPipelineStageFlags dstStageMask)
{
	VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
	barrier.dstAccessMask = dstAccessMask;
	barrier.image = image;
	barrier.newLayout = newLayout;
	barrier.oldLayout = oldLayout;
	barrier.srcAccessMask = srcAccessMask;
	barrier.subresourceRange.aspectMask = aspectFlag;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.layerCount = numLayers;
	barrier.subresourceRange.levelCount = 1;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

	auto commandBuffer = beginSingleTimeCommandBuffer();
	vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, 0, 0, nullptr, 0, nullptr, 1, &barrier);
	endSingleTimeCommandBuffer(commandBuffer);
}


VkDescriptorSetLayout Renderer::createDescriptorSetLayout()
{
	VkDescriptorSetLayout layout;

	std::array<VkDescriptorSetLayoutBinding, 3> descSetLayoutBinding = {};
	descSetLayoutBinding[0].binding = 0;
	descSetLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descSetLayoutBinding[0].descriptorCount = 1;
	descSetLayoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	descSetLayoutBinding[0].pImmutableSamplers = nullptr;

	descSetLayoutBinding[1].binding = 1;
	descSetLayoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descSetLayoutBinding[1].descriptorCount = 1;
	descSetLayoutBinding[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	descSetLayoutBinding[1].pImmutableSamplers = nullptr;

	descSetLayoutBinding[2].binding = 2;
	descSetLayoutBinding[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descSetLayoutBinding[2].descriptorCount = 1;
	descSetLayoutBinding[2].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	descSetLayoutBinding[2].pImmutableSamplers = nullptr;

	
	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutCreateInfo.bindingCount = static_cast<uint32_t>(descSetLayoutBinding.size());
	layoutCreateInfo.pBindings = descSetLayoutBinding.data();
	
	if (vkCreateDescriptorSetLayout(m_device, &layoutCreateInfo, nullptr, &layout) != VK_SUCCESS)
	{
		std::cerr << "Fail to create descriptor set layout." << std::endl;
		throw std::runtime_error("");
	}

	return layout;

}

VkDescriptorSetLayout Renderer::createSkyboxDescriptorSetLayout()
{
	VkDescriptorSetLayout layout;

	std::array<VkDescriptorSetLayoutBinding, 2> descSetLayoutBinding = {};
	descSetLayoutBinding[0].binding = 0;
	descSetLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descSetLayoutBinding[0].descriptorCount = 1;
	descSetLayoutBinding[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	descSetLayoutBinding[0].pImmutableSamplers = nullptr;


	descSetLayoutBinding[1].binding = 1;
	descSetLayoutBinding[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descSetLayoutBinding[1].descriptorCount = 1;
	descSetLayoutBinding[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	descSetLayoutBinding[1].pImmutableSamplers = nullptr;


	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutCreateInfo.bindingCount = static_cast<uint32_t>(descSetLayoutBinding.size());
	layoutCreateInfo.pBindings = descSetLayoutBinding.data();

	if (vkCreateDescriptorSetLayout(m_device, &layoutCreateInfo, nullptr, &layout) != VK_SUCCESS)
	{
		std::cerr << "Fail to create descriptor set layout." << std::endl;
		throw std::runtime_error("");
	}

	return layout;

}


VkDescriptorPool Renderer::createDescriptorPool()
{
	VkDescriptorPool pool;
	std::array<VkDescriptorPoolSize, 3> descPoolSzie = {};
	descPoolSzie[0].descriptorCount = n_buffers;
	descPoolSzie[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	descPoolSzie[1].descriptorCount = n_buffers;
	descPoolSzie[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

	descPoolSzie[2].descriptorCount = n_buffers;
	descPoolSzie[2].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;


	VkDescriptorPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	createInfo.poolSizeCount = static_cast<uint32_t>(descPoolSzie.size());
	createInfo.pPoolSizes = descPoolSzie.data();
	createInfo.maxSets = n_buffers;

	if (vkCreateDescriptorPool(m_device, &createInfo, nullptr, &pool) != VK_SUCCESS)
	{
		std::cerr << "Fail to create descriptor pool." << std::endl;
		throw std::runtime_error("");
	}

	return pool;
}



UniformBuffer* Renderer::createUniformBuffer()
{
	UniformBuffer* uBuffer = new UniformBuffer(this);

	VkDeviceSize boneTransformSize = sizeof(glm::mat4) * MAX_BONE_PER_SKELETON + sizeof(glm::mat4) * 2;
	VkDeviceSize lightPropertySize = sizeof(Light);

	for (unsigned int i = 0; i < n_buffers; i++)
	{
		createBuffer(uBuffer->m_boneTransform[i], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, boneTransformSize, uBuffer->m_boneTransformMemory[i], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		createBuffer(uBuffer->m_light[i], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, lightPropertySize, uBuffer->m_lightMemory[i], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}
	return uBuffer;
}



void Renderer::updateUniformBuffer(UniformBuffer& uBuffer, const std::vector<glm::mat4>& transformations, Light* light)
{
	//change the buffer of the image that is not being rendered now

	auto transformCopy = transformations;

	unsigned int writeToImageIdx = m_currentRenderingImgIdx ^ 1;


	void* data = nullptr;

	vkMapMemory(m_device, uBuffer.m_boneTransformMemory[writeToImageIdx], 0, sizeof(glm::mat4) * transformations.size(), 0, &data);

	memcpy(data, transformCopy.data(), sizeof(glm::mat4) * transformations.size());

	vkUnmapMemory(m_device, uBuffer.m_boneTransformMemory[writeToImageIdx]);

	if (light != nullptr)
	{
		data = nullptr;

		vkMapMemory(m_device, uBuffer.m_lightMemory[writeToImageIdx], 0, sizeof(Light), 0, &data);
		memcpy(data, light, sizeof(Light));
		vkUnmapMemory(m_device, uBuffer.m_lightMemory[writeToImageIdx]);
	}


}

CubeMap* Renderer::createCubeMap(std::string texturePaths[6])
{
	CubeMap* cubemap = new CubeMap(this);

	int cubeMapWidth, cubeMapHeight, cubeMapNChannel;
	stbi_uc* front = stbi_load(texturePaths[0].c_str(), &cubeMapWidth, &cubeMapHeight, &cubeMapNChannel, STBI_rgb_alpha);

	if (front == nullptr) throw std::runtime_error("");


	VkDeviceSize imageSize = cubeMapWidth * cubeMapHeight * 4;

	VkDeviceSize bufferSize = imageSize * 6;

	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMem;
	createBuffer(stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, bufferSize, stagingBufferMem, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	stbi_image_free(front);


	void* data;
	vkMapMemory(m_device, stagingBufferMem, 0, bufferSize, 0, &data);

	for (int i = 0; i < 6; i++)
	{
		int width, height, nChannel;
		stbi_uc* textureImage = stbi_load(texturePaths[i].c_str(), &width, &height, &nChannel, STBI_rgb_alpha);
		if (textureImage == nullptr || width != cubeMapWidth || height != cubeMapHeight) throw std::runtime_error("");
	
		memcpy(static_cast<char*>(data) + i*imageSize, reinterpret_cast<const void*>(textureImage), static_cast<size_t>(imageSize));
	
		stbi_image_free(textureImage);
	}

	vkUnmapMemory(m_device, stagingBufferMem);



	createImage(cubemap->m_cubeMapImage,
		cubemap->m_cubeMapImageMemory,
		static_cast<uint32_t>(cubeMapWidth),
		static_cast<uint32_t>(cubeMapHeight),
		1,
		6,
		1,
		VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
		VK_IMAGE_TYPE_2D,
		VK_FORMAT_R8G8B8A8_UNORM,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	transitImageLayout(cubemap->m_cubeMapImage, 6, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0, VK_ACCESS_TRANSFER_WRITE_BIT, VK_IMAGE_ASPECT_COLOR_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);

	VkBufferImageCopy region = {};
	region.bufferImageHeight = 0;
	region.bufferOffset = 0;
	region.bufferRowLength = 0;

	region.imageExtent = { static_cast<uint32_t>(cubeMapWidth), static_cast<uint32_t>(cubeMapHeight), 1 };
	region.imageOffset = { 0,0,0 };
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 6;
	region.imageSubresource.mipLevel = 0;

	auto commandBuffer = beginSingleTimeCommandBuffer();
	vkCmdCopyBufferToImage(commandBuffer, stagingBuffer, cubemap->m_cubeMapImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	endSingleTimeCommandBuffer(commandBuffer);

	transitImageLayout(cubemap->m_cubeMapImage, 6, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_IMAGE_ASPECT_COLOR_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMem, nullptr);

	cubemap->m_view = createImageView(cubemap->m_cubeMapImage, 6, VK_IMAGE_ASPECT_COLOR_BIT, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_VIEW_TYPE_CUBE);

	cubemap->m_sampler = createTextureSampler();


	return cubemap;
}

//this function is different from the previous one by that it modified all of ubo in tBuffer
void Renderer::initializeUniformBuffer(UniformBuffer& uBuffer, const std::vector<glm::mat4>& transformations, Light* light)
{
	//change the buffer of the image that is not being rendered now

	auto transformCopy = transformations;

	for (unsigned int i = 0; i < n_buffers; i++)
	{
		void* data = nullptr;

		vkMapMemory(m_device, uBuffer.m_boneTransformMemory[i], 0, sizeof(glm::mat4) * transformations.size(), 0, &data);

		memcpy(data, transformCopy.data(), sizeof(glm::mat4) * transformations.size());

		vkUnmapMemory(m_device, uBuffer.m_boneTransformMemory[i]);

		if (light != nullptr)
		{
			data = nullptr;

			vkMapMemory(m_device, uBuffer.m_lightMemory[i], 0, sizeof(Light), 0, &data);

			memcpy(data, light, sizeof(Light));

			vkUnmapMemory(m_device, uBuffer.m_lightMemory[i]);
		}
	}



}


void Renderer::createRenderPass()
{
	
	VkAttachmentDescription attachmentDesc = {};
	attachmentDesc.format = m_imageFormat;
	attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDesc = {};
	subpassDesc.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDesc.colorAttachmentCount = 1;
	subpassDesc.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; 
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo createInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO };
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &attachmentDesc;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpassDesc;
	createInfo.dependencyCount = 1;
	createInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(m_device, &createInfo, nullptr, &m_renderPass) != VK_SUCCESS)
	{
		std::cerr << "Fail to create render pass." << std::endl;
		throw std::runtime_error("");
	}

}



VkImageView Renderer::createImageView(VkImage& image, uint32_t numLayers, VkImageAspectFlags aspectMask, VkFormat viewFormat, VkImageViewType viewType)
{
	VkImageView view;
	VkImageSubresourceRange range = {};
	range.aspectMask = aspectMask;
	range.baseArrayLayer = 0;
	range.baseMipLevel = 0;
	range.layerCount = numLayers;
	range.levelCount = 1;
	VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	createInfo.format = viewFormat;
	createInfo.image = image;
	createInfo.subresourceRange = range;
	createInfo.viewType = viewType;

	if (vkCreateImageView(m_device, &createInfo, nullptr, &view) != VK_SUCCESS)
	{
		std::cerr << "Fail to create image view." << std::endl;
		throw std::runtime_error("");
	}

	return view;

}

VkSampler Renderer::createTextureSampler()
{
	VkSampler sampler;
	VkSamplerCreateInfo createInfo = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
	createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	createInfo.anisotropyEnable = VK_TRUE;
	createInfo.maxAnisotropy = 16;
	createInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
	createInfo.compareEnable = VK_FALSE;
	createInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	createInfo.magFilter = VK_FILTER_LINEAR;
	createInfo.minFilter = VK_FILTER_LINEAR;
	createInfo.maxLod = 0.0f;
	createInfo.minLod = 0.0f;
	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	createInfo.mipLodBias = 0.0f;
	createInfo.unnormalizedCoordinates = VK_FALSE;

	if (vkCreateSampler(m_device, &createInfo, nullptr, &sampler) != VK_SUCCESS)
	{
		std::cerr << "Fail to create texture sampler." << std::endl;
		throw std::runtime_error("");
	}
	return sampler;
}

void Renderer::cleanup()
{

	vkDeviceWaitIdle(m_device);

	for (const auto& framebuffer : m_framebuffers)
	{
		vkDestroyFramebuffer(m_device, framebuffer, nullptr);
	}

	vkDestroyRenderPass(m_device, m_renderPass, nullptr);



	for (const auto& imageView : m_imageViews)
	{
		vkDestroyImageView(m_device, imageView, nullptr);
	}

	for (uint32_t i = 0; i < n_buffers; i++)
	{
		
		vkDestroySemaphore(m_device, m_graphicsFinishSemaphores[i], nullptr);
		vkDestroyFence(m_device, m_imageFinishRenderFence[i], nullptr);
	}
	
	vkDestroyFence(m_device, m_acquireImageFence, nullptr);

	vkDestroyCommandPool(m_device, m_commandPool, nullptr);

	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

	vkDestroyDevice(m_device, nullptr);

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

	vkDestroyInstance(m_instance,nullptr);
	
	glfwDestroyWindow(m_window);

	glfwTerminate();
}



std::array<VkDescriptorSet, n_buffers> Renderer::createDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorPool pool, const UniformBuffer& uBuffer, const Texture& texture)
{
	std::array<VkDescriptorSet, n_buffers> set;

	std::vector<VkDescriptorSetLayout> layouts(n_buffers, layout);

	VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = n_buffers;
	allocInfo.pSetLayouts = layouts.data();


	VkResult t;
	if ( (t = vkAllocateDescriptorSets(m_device, &allocInfo, set.data())) != VK_SUCCESS)
	{
		std::cerr << "Fail to create descriptor sets." << std::endl;
		throw std::runtime_error("");
	}

	for (uint32_t i = 0; i < n_buffers; i++)
	{
		VkDescriptorBufferInfo boneTransformBufferInfo = {};
		boneTransformBufferInfo.offset = 0;
		boneTransformBufferInfo.buffer = uBuffer.m_boneTransform[i];
		boneTransformBufferInfo.range = VK_WHOLE_SIZE;

		VkDescriptorBufferInfo lightInfo = {};
		lightInfo.offset = 0;
		lightInfo.buffer = uBuffer.m_light[i];
		lightInfo.range = VK_WHOLE_SIZE;

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = texture.m_view;
		imageInfo.sampler = texture.m_sampler;

		std::array<VkWriteDescriptorSet, 3> writeDescSet = {};
		writeDescSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescSet[0].descriptorCount = 1;
		writeDescSet[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescSet[0].dstArrayElement = 0;
		writeDescSet[0].dstBinding = 0;
		writeDescSet[0].dstSet = set[i];
		writeDescSet[0].pBufferInfo = &boneTransformBufferInfo;

		writeDescSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescSet[1].descriptorCount = 1;
		writeDescSet[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescSet[1].dstArrayElement = 0;
		writeDescSet[1].dstBinding = 1;
		writeDescSet[1].dstSet = set[i];
		writeDescSet[1].pImageInfo = &imageInfo;

		writeDescSet[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescSet[2].descriptorCount = 1;
		writeDescSet[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescSet[2].dstArrayElement = 0;
		writeDescSet[2].dstBinding = 2;
		writeDescSet[2].dstSet = set[i];
		writeDescSet[2].pBufferInfo = &lightInfo;

		vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writeDescSet.size()), writeDescSet.data(), 0, nullptr);
	}

	return set;
}

std::array<VkDescriptorSet, n_buffers> Renderer::createSkyboxDescriptorSet(VkDescriptorSetLayout layout, VkDescriptorPool pool, const UniformBuffer& uBuffer, const CubeMap& cubemap)
{
	std::array<VkDescriptorSet, n_buffers> set;

	std::vector<VkDescriptorSetLayout> layouts(n_buffers, layout);

	VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = pool;
	allocInfo.descriptorSetCount = n_buffers;
	allocInfo.pSetLayouts = layouts.data();


	VkResult t;
	if ((t = vkAllocateDescriptorSets(m_device, &allocInfo, set.data())) != VK_SUCCESS)
	{
		std::cerr << "Fail to create descriptor sets." << std::endl;
		throw std::runtime_error("");
	}

	for (uint32_t i = 0; i < n_buffers; i++)
	{

		VkDescriptorImageInfo imageInfo = {};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = cubemap.m_view;
		imageInfo.sampler = cubemap.m_sampler;

		VkDescriptorBufferInfo lightInfo = {};
		lightInfo.offset = 0;
		lightInfo.buffer = uBuffer.m_light[i];
		lightInfo.range = VK_WHOLE_SIZE;

	
		std::array<VkWriteDescriptorSet, 2> writeDescSet = {};

		writeDescSet[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescSet[0].descriptorCount = 1;
		writeDescSet[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		writeDescSet[0].dstArrayElement = 0;
		writeDescSet[0].dstBinding = 0;
		writeDescSet[0].dstSet = set[i];
		writeDescSet[0].pImageInfo = &imageInfo;

		writeDescSet[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeDescSet[1].descriptorCount = 1;
		writeDescSet[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescSet[1].dstArrayElement = 0;
		writeDescSet[1].dstBinding = 1;
		writeDescSet[1].dstSet = set[i];
		writeDescSet[1].pBufferInfo = &lightInfo;

		vkUpdateDescriptorSets(m_device, static_cast<uint32_t>(writeDescSet.size()), writeDescSet.data(), 0, nullptr);
	}

	return set;
}

size_t Renderer::getNumActors()
{
	return m_actors.size();
}

void Renderer::setTerrain(Terrain* terrain)
{
	if (m_terrain != nullptr) delete m_terrain;
	m_terrain = terrain;
}

uint32_t Renderer::findSuitableMemType(uint32_t filter, VkMemoryPropertyFlags properties)
{
	for (uint32_t i = 0; i < m_physicalDeviceMemProp.memoryTypeCount; i++)
	{
		if ((filter & (1 << i) && ((m_physicalDeviceMemProp.memoryTypes[i].propertyFlags & properties) == properties)))
		{
			return i;
		}
	}

	std::cerr << "Fail to find suitable memory type!" << std::endl;
	throw std::runtime_error("");
}

Renderer::~Renderer()
{
	cleanup();
}

GLFWwindow** Renderer::getWindow()
{
	return &m_window;
}

VertexBuffer::VertexBuffer(Renderer* renderer)
{
	m_renderer = renderer;
	m_vertexBuffer = VK_NULL_HANDLE;
	m_vertexBufferMemory = VK_NULL_HANDLE;
}

VertexBuffer::~VertexBuffer()
{
	vkDeviceWaitIdle(m_renderer->getDevice());
	vkDestroyBuffer(m_renderer->getDevice(), m_vertexBuffer, nullptr);
	vkFreeMemory(m_renderer->getDevice(), m_vertexBufferMemory, nullptr);
}

IndexBuffer::IndexBuffer(Renderer* renderer)
{
	m_renderer = renderer;
}

IndexBuffer::~IndexBuffer()
{
	vkDeviceWaitIdle(m_renderer->getDevice());
	vkDestroyBuffer(m_renderer->getDevice(), m_indexBuffer, nullptr);
	vkFreeMemory(m_renderer->getDevice(), m_indexBufferMemory, nullptr);
}

Texture::Texture(Renderer* renderer)
{
	m_renderer = renderer;
}

Texture::~Texture()
{

	vkDeviceWaitIdle(m_renderer->getDevice());
	vkDestroyImageView(m_renderer->getDevice(), m_view, nullptr);
	vkDestroyImage(m_renderer->getDevice(), m_textureImage, nullptr);
	vkFreeMemory(m_renderer->getDevice(), m_textureImageMemory, nullptr);
	vkDestroySampler(m_renderer->getDevice(), m_sampler, nullptr);
}

UniformBuffer::UniformBuffer(Renderer* renderer)
{
	m_renderer = renderer;
}

UniformBuffer::~UniformBuffer()
{
	vkDeviceWaitIdle(m_renderer->getDevice());
	for (unsigned int i = 0; i < n_buffers; i++)
	{
		vkDestroyBuffer(m_renderer->getDevice(), m_boneTransform[i], nullptr);
		vkFreeMemory(m_renderer->getDevice(), m_boneTransformMemory[i], nullptr);

		vkDestroyBuffer(m_renderer->getDevice(), m_light[i], nullptr);
		vkFreeMemory(m_renderer->getDevice(), m_lightMemory[i], nullptr);
	}
}

DataDescription::DataDescription(Renderer* renderer)
{
	m_renderer = renderer;
}

DataDescription::~DataDescription()
{
	vkDeviceWaitIdle(m_renderer->getDevice());
	vkDestroyDescriptorPool(m_renderer->getDevice(), m_pool, nullptr);
	vkDestroyDescriptorSetLayout(m_renderer->getDevice(), m_layout, nullptr);
}

CubeMap::CubeMap(Renderer* renderer)
{
	m_renderer = renderer;
}

CubeMap::~CubeMap()
{
	vkDeviceWaitIdle(m_renderer->getDevice());
	vkDestroyImageView(m_renderer->getDevice(), m_view, nullptr);
	vkDestroySampler(m_renderer->getDevice(), m_sampler, nullptr);
	vkDestroyImage(m_renderer->getDevice(), m_cubeMapImage, nullptr);
	vkFreeMemory(m_renderer->getDevice(), m_cubeMapImageMemory, nullptr);
}

Pipeline::Pipeline(Renderer* renderer)
{
	m_renderer = renderer;
}

Pipeline::~Pipeline()
{
	vkDeviceWaitIdle(m_renderer->getDevice());
	vkDestroyPipeline(m_renderer->getDevice(), m_pipeline, nullptr);
	vkDestroyPipelineLayout(m_renderer->getDevice(), m_pipelineLayout, nullptr);
}

Material::Material()
{
}

Material::~Material()
{
	if(m_texture != nullptr) delete m_texture;
	if(m_cubeMap != nullptr) delete m_cubeMap;
}
