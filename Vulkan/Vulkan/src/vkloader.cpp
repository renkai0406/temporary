#include "pch.h"
#include "vkloader.h"

VulkanLoader::VulkanLoader()
{
}


VulkanLoader::~VulkanLoader()
{
}

void VulkanLoader::init(const std::string& title, GLFWwindow* glfwWin)
{
	createInstance(title);
	setupDebugMessenger();
	createSurface(glfwWin);
	pickPhysicalDevice();
	checkQueueFamily();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createReanderPass();
	createPippline(SHADER_PATH, SHADER_COUNT);
	
	/*createCommandPool();
	createCommandBuffer();*/
}

void VulkanLoader::clearup()
{
	//vkFreeCommandBuffers(vkInfo.device, vkInfo.cpool, 1, &vkInfo.cbuffer);
	
	//vkDestroyCommandPool(vkInfo.device, vkInfo.cpool, NULL);

	vkDestroyPipelineLayout(vkInfo.device, vkInfo.pipelineLayout, nullptr);

	vkDestroyRenderPass(vkInfo.device, vkInfo.renderPass, nullptr);

	for (auto imageView : vkInfo.swapImgViews) {
		vkDestroyImageView(vkInfo.device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(vkInfo.device, vkInfo.swapchain, nullptr);

	vkDeviceWaitIdle(vkInfo.device);
	vkDestroyDevice(vkInfo.device, NULL);

	vkDestroySurfaceKHR(vkInfo.instance, vkInfo.surface, nullptr);

	if (vkInfo.layersEnabled) {
		DestroyDebugUtilsMessengerEXT(vkInfo.instance, vkInfo.debugMessenger, nullptr);
	}

	vkDestroyInstance(vkInfo.instance, NULL);
}

void VulkanLoader::createInstance(const std::string& title)
{

	AppManager::appAssert(!vkInfo.layersEnabled || vkInfo.layersEnabled && checkLayersSupport(), "validation layers requested, but not available!");

	VkApplicationInfo appInfo = {};
	appInfo.pApplicationName = title.c_str();
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;


	VkInstanceCreateInfo icInfo = {};
	icInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	icInfo.pApplicationInfo = &appInfo;

	if (vkInfo.layersEnabled) {
		icInfo.enabledLayerCount = static_cast<uint32_t>(vkInfo.layers.size());
		icInfo.ppEnabledLayerNames = vkInfo.layers.data();
	}
	else {
		icInfo.enabledLayerCount = 0;
	}

	//acquire the extensition data.
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	
	//add debug utils extension.
	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
	if (vkInfo.layersEnabled)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	AppManager::appAssert(checkExtensionsSupport(extensions), "there exit some extensions required that can't be supported.");

	//fill the extension data.
	icInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	icInfo.ppEnabledExtensionNames = extensions.data();

	VkResult result = vkCreateInstance(&icInfo, NULL, &vkInfo.instance);
	AppManager::appAssert(result == VK_SUCCESS, "someting wrong happened when creating the vulkan instance.");

}

void VulkanLoader::setupDebugMessenger()
{
	if (!vkInfo.layersEnabled)return;
	VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;
	VkResult result = CreateDebugUtilsMessengerEXT(vkInfo.instance, &createInfo, nullptr, &vkInfo.debugMessenger);
	
	AppManager::appAssert(result == VK_SUCCESS, "failed to set up debug messenger.");

}

void VulkanLoader::createSurface(GLFWwindow* glfwWin)
{
	VkResult result = glfwCreateWindowSurface(vkInfo.instance, glfwWin, NULL, &vkInfo.surface);
	AppManager::appAssert(result == VK_SUCCESS, "failed to create window surface.");
}

void VulkanLoader::pickPhysicalDevice()
{
	unsigned int gpuCount;
	// Get the number of devices (GPUs) available.
	VkResult res = vkEnumeratePhysicalDevices(vkInfo.instance, &gpuCount, NULL);
	AppManager::appAssert(gpuCount >= GPU_NEEDED_COUNT, "the number of gpu is less than needed.");
	// Allocate space and get the list of devices.
	std::vector<VkPhysicalDevice> gpus(gpuCount);
	res = vkEnumeratePhysicalDevices(vkInfo.instance, &gpuCount, gpus.data());
	AppManager::appAssert(res == VK_SUCCESS, "something bad happened when enumerating physic devices.");

	std::multimap<int, VkPhysicalDevice> candidates;
	for (auto gpu : gpus) {
		int score = rateDeviceSuitability(gpu);
		candidates.insert(std::make_pair(score, gpu));
	}

	if (candidates.rbegin()->first > 0) {
		vkInfo.gpu = candidates.rbegin()->second;
	}
	else {
		AppManager::appError("failed to find a suitable GPU.");
	}

}

void VulkanLoader::checkQueueFamily()
{
	unsigned int i, queueFamilyCount;
	const VkPhysicalDevice& gpu = vkInfo.gpu;

	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, NULL);
	AppManager::appAssert(queueFamilyCount > 0, "there is no queue family found in this gpu.");

	std::vector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueFamilyProps.data());

	bool hasPresent = false, hasGraph = false;

	for (i = 0; i < queueFamilyCount; i++)
	{

		if (!hasGraph & queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			vkInfo.graQueueFamilyIndex = i;
			hasGraph = true;
		}

		if (!hasPresent)
		{
			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(vkInfo.gpu, i, vkInfo.surface, &presentSupport);
			if (presentSupport) {
				vkInfo.preQueueFamilyIndex = i;
				hasPresent = true;
			}
		}

		if (hasPresent && hasGraph)
			break;

		if (i == queueFamilyCount - 1)
		{
			AppManager::appError("failed to find a suitable queue family.");
		}
	}
}

void VulkanLoader::createLogicalDevice()
{
	float queuePriorityes[] = { 0.0f };
	
	std::vector< VkDeviceQueueCreateInfo> dqcInfos;
	std::set<uint32_t> uniqueQueueFamilies = { vkInfo.graQueueFamilyIndex, vkInfo.preQueueFamilyIndex };
	for (const auto family : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo qcInfo = {};
		qcInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		qcInfo.queueCount = 1;
		qcInfo.pQueuePriorities = queuePriorityes;
		qcInfo.queueFamilyIndex = family;
		dqcInfos.push_back(qcInfo);
	}

	VkDeviceCreateInfo devInfo = {};
	devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	devInfo.queueCreateInfoCount = static_cast<uint32_t>(dqcInfos.size());
	devInfo.pQueueCreateInfos = dqcInfos.data();

	if (vkInfo.layersEnabled) {
		devInfo.enabledLayerCount = static_cast<uint32_t>(vkInfo.layers.size());
		devInfo.ppEnabledLayerNames = vkInfo.layers.data();
	}
	else {
		devInfo.enabledLayerCount = 0;
	}

	devInfo.enabledExtensionCount = static_cast<uint32_t>(vkInfo.deviceExtensions.size());
	devInfo.ppEnabledExtensionNames = vkInfo.deviceExtensions.data();

	VkResult result = vkCreateDevice(vkInfo.gpu, &devInfo, NULL, &vkInfo.device);
	AppManager::appAssert(result == VK_SUCCESS, "something bad happened when creating device.");
}

void VulkanLoader::createSwapChain()
{
	VkSurfaceCapabilitiesKHR cap;
	VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkInfo.gpu, vkInfo.surface, &cap);
	AppManager::appAssert(result == VK_SUCCESS, "cann't acquire surface capabilities in current physical device.");

	VkFormat format;
	uint32_t formatCount;
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(vkInfo.gpu, vkInfo.surface, &formatCount, NULL);
	AppManager::appAssert(result == VK_SUCCESS, "something badly happened when acquiring count of surface formats.");
	VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(vkInfo.gpu, vkInfo.surface, &formatCount, surfFormats);
	AppManager::appAssert(result == VK_SUCCESS, "something badly happened when acquiring surface formats.");
	if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
		format = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else {
		AppManager::appAssert(formatCount >= 1, "there exits no format in current physical device");
		format = surfFormats[0].format;
	}
	free(surfFormats);

	vkInfo.scFormat = format;

	VkExtent2D swapchainExtent;
	// width and height are either both 0xFFFFFFFF, or both not 0xFFFFFFFF.
	if (cap.currentExtent.width == 0xFFFFFFFF) {
		// If the surface size is undefined, the size is set to
		// the size of the images requested.
		swapchainExtent.width = vkInfo.width;
		swapchainExtent.height = vkInfo.height;
		if (swapchainExtent.width < cap.minImageExtent.width) {
			swapchainExtent.width = cap.minImageExtent.width;
		}
		else if (swapchainExtent.width > cap.maxImageExtent.width) {
			swapchainExtent.width = cap.maxImageExtent.width;
		}

		if (swapchainExtent.height < cap.minImageExtent.height) {
			swapchainExtent.height = cap.minImageExtent.height;
		}
		else if (swapchainExtent.height > cap.maxImageExtent.height) {
			swapchainExtent.height = cap.maxImageExtent.height;
		}
	}
	else {
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = cap.currentExtent;
	}

	vkInfo.scExtent = swapchainExtent;

	VkSurfaceTransformFlagBitsKHR preTransform;
	if (cap.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
		preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else {
		preTransform = cap.currentTransform;
	}

	VkSwapchainCreateInfoKHR sccInfo = {};
	sccInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	sccInfo.surface = vkInfo.surface;
	sccInfo.imageFormat = format;
	sccInfo.minImageCount = cap.minImageCount;
	sccInfo.imageArrayLayers = 1;
	sccInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	sccInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
	sccInfo.imageExtent.width = swapchainExtent.width;
	sccInfo.imageExtent.height = swapchainExtent.height;
	sccInfo.preTransform = preTransform;
	sccInfo.oldSwapchain = VK_NULL_HANDLE;
	sccInfo.clipped = true;
	sccInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
	sccInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	sccInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	sccInfo.queueFamilyIndexCount = 0;
	sccInfo.pQueueFamilyIndices = nullptr;
	if (vkInfo.graQueueFamilyIndex != vkInfo.preQueueFamilyIndex) {
		uint32_t queueFamilyIndices[2] = 
		{
			(uint32_t)vkInfo.graQueueFamilyIndex,
			(uint32_t)vkInfo.preQueueFamilyIndex 
		};
		sccInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		sccInfo.queueFamilyIndexCount = 2;
		sccInfo.pQueueFamilyIndices = queueFamilyIndices;
	}

	result = vkCreateSwapchainKHR(vkInfo.device, &sccInfo, nullptr, &vkInfo.swapchain);
	AppManager::appAssert(result == VK_SUCCESS, "something wrong happened when creating swap chain");

	uint32_t imageCount;
	vkGetSwapchainImagesKHR(vkInfo.device, vkInfo.swapchain, &imageCount, nullptr);
	vkInfo.swapImages.resize(imageCount);
	vkGetSwapchainImagesKHR(vkInfo.device, vkInfo.swapchain, &imageCount, vkInfo.swapImages.data());
}

void VulkanLoader::createImageViews()
{
	vkInfo.swapImgViews.resize(vkInfo.swapImages.size());

	unsigned int i = 0;
	
	for (i = 0; i < vkInfo.swapImages.size(); i++)
	{
		VkImageViewCreateInfo ivcInfo = {};
		ivcInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ivcInfo.image = vkInfo.swapImages[i];
		ivcInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ivcInfo.format = vkInfo.scFormat;
		ivcInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		ivcInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		ivcInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		ivcInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		ivcInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ivcInfo.subresourceRange.baseMipLevel = 0;
		ivcInfo.subresourceRange.levelCount = 1;
		ivcInfo.subresourceRange.baseArrayLayer = 0;
		ivcInfo.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(vkInfo.device, &ivcInfo, NULL, &vkInfo.swapImgViews[i]);
		AppManager::appAssert(result == VK_SUCCESS, "failed to create image view:" + std::to_string(i));
	}
}

void VulkanLoader::createReanderPass()
{
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = vkInfo.scFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VkResult result = vkCreateRenderPass(vkInfo.device, &renderPassInfo, nullptr, &vkInfo.renderPass);
	AppManager::appAssert(result == VK_SUCCESS, "failed to create render pass!");
}

void VulkanLoader::createPippline(const std::string* files, int count)
{
	auto vertModule = createShaderModule(readFile(files[0]));
	auto geomModule = createShaderModule(readFile(files[2]));

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = geomModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = vkInfo.scExtent.width;
	viewport.height = vkInfo.scExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.offset = { 0, 0 };
	scissor.extent = vkInfo.scExtent;

	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	VkPipelineMultisampleStateCreateInfo multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	VkDynamicState dynamicStates[] = {
	VK_DYNAMIC_STATE_VIEWPORT,
	VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicState = {};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = dynamicStates;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	VkResult result = vkCreatePipelineLayout(vkInfo.device, &pipelineLayoutInfo, nullptr, &vkInfo.pipelineLayout);
	AppManager::appAssert(result == VK_SUCCESS, "failed to create pipeline layout!");



	vkDestroyShaderModule(vkInfo.device, vertModule, nullptr);
	vkDestroyShaderModule(vkInfo.device, geomModule, nullptr);
}

/*void VulkanLoader::createCommandPool()
{
	VkCommandPoolCreateInfo cpInfo = {};
	cpInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cpInfo.queueFamilyIndex = vkInfo.graQueueFamilyIndex;

	VkResult result = vkCreateCommandPool(vkInfo.device, &cpInfo, NULL, &vkInfo.cpool);
	AppManager::appAssert(result == VK_SUCCESS, "something bad happened when creating command pool.");
}

void VulkanLoader::createCommandBuffer()
{
	VkCommandBufferAllocateInfo cbaInfo = {};
	cbaInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cbaInfo.commandBufferCount = 1;
	cbaInfo.commandPool = vkInfo.cpool;
	cbaInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkResult result = vkAllocateCommandBuffers(vkInfo.device, &cbaInfo, &vkInfo.cbuffer);
	AppManager::appAssert(result == VK_SUCCESS, "something bad happened when allocating command buffers.");
}*/

bool VulkanLoader::checkLayersSupport()
{
	//get all the layers that the insance supports.
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : vkInfo.layers) {
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

bool VulkanLoader::checkExtensionsSupport(const std::vector<const char*>& needs)
{
	unsigned int needsCount = needs.size();
	uint32_t supportsCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &supportsCount, nullptr);
	std::vector<VkExtensionProperties> sextensions(supportsCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &supportsCount, sextensions.data());

	std::vector<std::string> sextensionsNames;

	for(const auto& extension : sextensions)
	{
		sextensionsNames.insert(sextensionsNames.end(), extension.extensionName);
	}

	for (unsigned int i = 0; i < needsCount; i++)
	{
		auto it = std::find(sextensionsNames.begin(), sextensionsNames.end(), needs[i]);
		if (it == sextensionsNames.end())
			return false;
	}

	return true;
}

VKAPI_ATTR VkBool32 VKAPI_CALL VulkanLoader::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT * pCallbackData, void * pUserData)
{
	std::string msg = pCallbackData->pMessage;
	switch (messageSeverity)
	{
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
	{
		
		Log::Instance()->log("validation layer message:" + msg);
		break;
	}
	case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
		AppManager::appError(msg);
		break;
	default:
		break;
	}

	return VK_FALSE;
}

std::vector<char> VulkanLoader::readFile(const std::string & fname)
{
	if (fname.size() < 1)
		return std::vector<char>();
	std::ifstream file(fname, std::ios::ate | std::ios::binary);

	AppManager::appAssert(file.is_open(), "failed to open file:" + fname);

	unsigned int size = (unsigned int)file.tellg();
	std::vector<char> buffer(size);

	file.seekg(0);
	file.read(buffer.data(), size);

	file.close();

	return buffer;
}

VkResult VulkanLoader::CreateDebugUtilsMessengerEXT(const VkInstance &instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void VulkanLoader::DestroyDebugUtilsMessengerEXT(const VkInstance &instance, VkDebugUtilsMessengerEXT &debugMessenger, const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

int VulkanLoader::rateDeviceSuitability(VkPhysicalDevice & device)
{
	int score = 0;

	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 2000;

	score += deviceProperties.limits.maxImageDimension2D;

	if (!deviceFeatures.geometryShader) {
		return 0;
	}

	return score;
}

VkShaderModule VulkanLoader::createShaderModule(const std::vector<char>& codes)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = codes.size();
	createInfo.pCode = reinterpret_cast<const unsigned int*>(codes.data());

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(vkInfo.device, &createInfo, nullptr, &shaderModule);
	AppManager::appAssert(result != VK_SUCCESS, "failed to create shader module!");

	return shaderModule;
}






