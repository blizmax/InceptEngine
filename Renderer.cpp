#include "Renderer.h"


Renderer::Renderer()
{
	m_clearColor.float32[0] = 0.5f;
	m_clearColor.float32[1] = 0.0f;
	m_clearColor.float32[2] = 0.0f;
	m_clearColor.float32[3] = 1.0f;
	m_clearValue.color = m_clearColor;

	//m_mvp.model = glm::rotate(glm::mat4(1.0f), glm::radians(180+30.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));  //TODO


	m_mvp.model = glm::mat4(1.0);
	m_mvp.view = glm::lookAt(glm::vec3(0, 0,-400), glm::vec3(0,0,0), glm::vec3(0,1,0));

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

void Renderer::clearScreen()
{
}

void Renderer::drawTriangle(const std::vector<glm::mat4>& boneT)
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


	updateUniformBuffer(boneT);
	

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

void Renderer::resizeWindow(int width, int height)
{
	m_minimized = (width == 0 || height == 0);
	if (m_minimized)
	{
		return;
	}
	isChangingSize = true;
	vkDeviceWaitIdle(m_device);
	m_currentRenderingImgIdx = 0;
	m_windowWidth = width;
	m_windowHeight = height;
	cleaupSwapChain();
	recreateSwapChain();
	isChangingSize = false;
}

void Renderer::init()
{
	createWindow();

	createInstance();

	createSurface();
	
	createPhysicalDevice();

	createLogicalDevice();

	createSwapchain();

	createImageView();

	createCommandPool();

	createVertexBuffer(m_vertices);

	createIndexBuffer(m_indices);

	createUniformBuffer();

	createDescriptorSetLayout();

	createDescriptorPool();

	createDescriptorSets();

	createRenderPass();
	
	createGraphicsPipeline();

	createFramebuffer();

	createSemaphore();

	createCommandBuffers();

	recordCommandBuffers();
}

void Renderer::createWindow()
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);


	m_windowWidth = 800;
	m_windowHeight = 600;
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

	VkDeviceCreateInfo createInfo = { VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO };
	createInfo.queueCreateInfoCount = 1;
	createInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
	
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

void Renderer::createImageView()
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


void Renderer::recordCommandBuffers()
{
	VkClearColorValue clearColor = {};
	clearColor.float32[0] = 0.5f;
	clearColor.float32[1] = 0.0f;
	clearColor.float32[2] = 0.0f;
	clearColor.float32[3] = 1.0f;
	VkClearValue clearValue = {};
	clearValue.color = clearColor;

	for (size_t i = 0; i < m_commandBuffers.size(); i++)
	{
		VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
		if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS)
		{
			std::cerr << "Fail to begin record command buffer " << i << std::endl;
			throw std::runtime_error("");
		}


		VkRenderPassBeginInfo renderPassBeginInfo = { VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO };
		renderPassBeginInfo.renderPass = m_renderPass;
		renderPassBeginInfo.renderArea.offset = { 0,0 };
		renderPassBeginInfo.renderArea.extent = m_swapchainExtent;
		renderPassBeginInfo.framebuffer = m_framebuffers[i];
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearValue;

		vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);

		VkDeviceSize offset[] = { 0 };
		VkBuffer vertexBuffers[] = { m_vertexBuffer };
		vkCmdBindVertexBuffers(m_commandBuffers[i], 0, 1, vertexBuffers, offset);

		vkCmdBindIndexBuffer(m_commandBuffers[i], m_indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		vkCmdBindDescriptorSets(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSets[i], 0, nullptr);

		vkCmdPushConstants(m_commandBuffers[i], m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MVP), (void*)&m_mvp);

		vkCmdDrawIndexed(m_commandBuffers[i], (uint32_t)m_indices.size(), 1, 0, 0, 0);

		vkCmdEndRenderPass(m_commandBuffers[i]);
		
		
		if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS)
		{
			std::cerr << "Fail to end record command buffer " << i << std::endl;
			throw std::runtime_error("");
		}

	}
}

void Renderer::createSemaphore()
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

void Renderer::createGraphicsPipeline()
{
	
	//shaders
	VkShaderModule vertexShader = loadShaderModule("D:\\Inception\\Content\\Shaders\\spv\\vertex.spv");
	VkShaderModule fragmentShader = loadShaderModule("D:\\Inception\\Content\\Shaders\\spv\\fragment.spv");

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
	inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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
	pipelineLayoutCreateInfo.pSetLayouts = &m_descriptorSetLayout;
	if (vkCreatePipelineLayout(m_device, &pipelineLayoutCreateInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
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
	graphicsPipelineCreateInfo.layout = m_pipelineLayout;
	graphicsPipelineCreateInfo.renderPass = m_renderPass;
	graphicsPipelineCreateInfo.subpass = 0; //index of the subpass that this graphics pipeline will use
	
	if (vkCreateGraphicsPipelines(m_device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, nullptr, &m_graphicsPipeline) != VK_SUCCESS)
	{
		std::cerr << "Fail to create graphics pipeline!" << std::endl;
		throw std::runtime_error("");
	}
	
	

	vkDestroyShaderModule(m_device, vertexShader, nullptr);
	vkDestroyShaderModule(m_device, fragmentShader, nullptr);

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

	//pipeline
	vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);

	//pipelinelayout
	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);

	//renderpass
	vkDestroyRenderPass(m_device, m_renderPass, nullptr);

	//swapchian
	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

}

void Renderer::recreateSwapChain()
{
	createSwapchain();

	createImageView();

	createRenderPass();

	createGraphicsPipeline();

	createFramebuffer();

	createSemaphore();

	createCommandBuffers();

	recordCommandBuffers();
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

void Renderer::createVertexBuffer(const std::vector<Vertex>& vertices)
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMem;
	VkDeviceSize size = sizeof(Vertex) * vertices.size();

	createBuffer(stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, stagingBufferMem, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data = nullptr;
	vkMapMemory(m_device, stagingBufferMem, 0, size, 0, &data);

	memcpy(data, vertices.data(), (size_t)size);

	vkUnmapMemory(m_device, stagingBufferMem);

	createBuffer(m_vertexBuffer, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, size, m_vertexBufferMem, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	copyBuffer(stagingBuffer, m_vertexBuffer, size);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);

	vkFreeMemory(m_device, stagingBufferMem, nullptr);
	
}

void Renderer::createIndexBuffer(const std::vector<uint32_t>& indices)
{
	VkDeviceSize size = indices.size() * sizeof(uint32_t);
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMem;
	createBuffer(stagingBuffer, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, stagingBufferMem, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(m_device, stagingBufferMem, 0, size, 0, &data);
	memcpy(data, indices.data(), size);
	vkUnmapMemory(m_device, stagingBufferMem);

	createBuffer(m_indexBuffer, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, size, m_indexBufferMem, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	copyBuffer(stagingBuffer, m_indexBuffer, size);

	vkDestroyBuffer(m_device, stagingBuffer, nullptr);
	vkFreeMemory(m_device, stagingBufferMem, nullptr);

}

void Renderer::updateFrame()
{

}

void Renderer::createDescriptorSetLayout()
{
	std::array<VkDescriptorSetLayoutBinding, 1> descSetLayoutBinding = {};
	descSetLayoutBinding[0].binding = 0;
	descSetLayoutBinding[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descSetLayoutBinding[0].descriptorCount = 1;
	descSetLayoutBinding[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	descSetLayoutBinding[0].pImmutableSamplers = nullptr;

	
	VkDescriptorSetLayoutCreateInfo layoutCreateInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	layoutCreateInfo.bindingCount = (uint32_t)descSetLayoutBinding.size();
	layoutCreateInfo.pBindings = descSetLayoutBinding.data();
	
	if (vkCreateDescriptorSetLayout(m_device, &layoutCreateInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
	{
		std::cerr << "Fail to create descriptor set layout." << std::endl;
		throw std::runtime_error("");
	}

}

void Renderer::createDescriptorPool()
{
	VkDescriptorPoolSize descPoolSzie = {};
	descPoolSzie.descriptorCount = n_buffers;
	descPoolSzie.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

	VkDescriptorPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO };
	createInfo.poolSizeCount = 1;
	createInfo.pPoolSizes = &descPoolSzie;
	createInfo.maxSets = n_buffers;

	if (vkCreateDescriptorPool(m_device, &createInfo, nullptr, &m_descriptorSetPool) != VK_SUCCESS)
	{
		std::cerr << "Fail to create descriptor pool." << std::endl;
		throw std::runtime_error("");
	}
}

void Renderer::createDescriptorSets()
{
	std::vector<VkDescriptorSetLayout> layouts(n_buffers, m_descriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
	allocInfo.descriptorPool = m_descriptorSetPool;
	allocInfo.descriptorSetCount = n_buffers;
	allocInfo.pSetLayouts = layouts.data();


	m_descriptorSets.resize(n_buffers);
	if (vkAllocateDescriptorSets(m_device, &allocInfo, m_descriptorSets.data()) != VK_SUCCESS)
	{
		std::cerr << "Fail to create descriptor sets." << std::endl;
		throw std::runtime_error("");
	}

	for (uint32_t i = 0; i < n_buffers; i++)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.offset = 0;
		bufferInfo.buffer = m_uniformBuffers[i];
		bufferInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet writeDescSet = { VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
		writeDescSet.descriptorCount = 1;
		writeDescSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		writeDescSet.dstArrayElement = 0;
		writeDescSet.dstBinding = 0;
		writeDescSet.dstSet = m_descriptorSets[i];
		writeDescSet.pBufferInfo = &bufferInfo;
		vkUpdateDescriptorSets(m_device, 1, &writeDescSet, 0, nullptr);
	}
}

void Renderer::createUniformBuffer()
{
	VkDeviceSize boneTransformBufferSize = sizeof(glm::mat4) * MAX_BONE_PER_SKELETON;
	m_uniformBuffers.resize(n_buffers);
	m_uniformBufferMem.resize(n_buffers);
	for (unsigned int i = 0; i < m_uniformBuffers.size(); i++)
	{
		createBuffer(m_uniformBuffers[i], VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, boneTransformBufferSize, m_uniformBufferMem[i], VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	}

}

void Renderer::updateUniformBuffer(const std::vector<glm::mat4>& boneTransforms)
{
	//change the buffer of the image that is not being rendered now

	auto transformCopy = boneTransforms;

	void* data = nullptr;

	unsigned int writeToImageIdx = m_currentRenderingImgIdx ^ 1;

	vkMapMemory(m_device, m_uniformBufferMem[writeToImageIdx], 0, sizeof(glm::mat4) * boneTransforms.size(), 0, &data);

	memcpy(data, transformCopy.data(), sizeof(glm::mat4) * boneTransforms.size());

	vkUnmapMemory(m_device, m_uniformBufferMem[writeToImageIdx]);

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



void Renderer::cleanup()
{

	vkDeviceWaitIdle(m_device);

	for (unsigned int i = 0; i < m_uniformBuffers.size(); i++)
	{
		vkDestroyBuffer(m_device, m_uniformBuffers[i], nullptr);
		vkFreeMemory(m_device, m_uniformBufferMem[i], nullptr);
	}

	vkDestroyDescriptorPool(m_device, m_descriptorSetPool, nullptr);

	vkDestroyBuffer(m_device, m_indexBuffer, nullptr);

	vkFreeMemory(m_device, m_indexBufferMem, nullptr);

	vkDestroyBuffer(m_device, m_vertexBuffer, nullptr);

	vkFreeMemory(m_device, m_vertexBufferMem, nullptr);

	for (const auto& framebuffer : m_framebuffers)
	{
		vkDestroyFramebuffer(m_device, framebuffer, nullptr);
	}

	vkDestroyRenderPass(m_device, m_renderPass, nullptr);

	vkDestroyPipelineLayout(m_device, m_pipelineLayout, nullptr);

	vkDestroyPipeline(m_device, m_graphicsPipeline, nullptr);

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

	vkDestroyDescriptorSetLayout(m_device, m_descriptorSetLayout, nullptr);

	vkDestroyCommandPool(m_device, m_commandPool, nullptr);

	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);

	vkDestroyDevice(m_device, nullptr);

	vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

	vkDestroyInstance(m_instance,nullptr);
	
	glfwDestroyWindow(m_window);

	glfwTerminate();
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

