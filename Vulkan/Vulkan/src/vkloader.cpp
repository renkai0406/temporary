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
	pickPhysicalDevice();
	checkQueuiFamily();
	createLogicalDevice();
	createSurface(glfwWin);
	createSwapChain();
	
	createCommandPool();
	createCommandBuffer();
}

void VulkanLoader::clearup()
{
	vkFreeCommandBuffers(vkInfo.device, vkInfo.cpool, 1, &vkInfo.cbuffer);
	
	vkDestroyCommandPool(vkInfo.device, vkInfo.cpool, NULL);

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

void VulkanLoader::checkQueuiFamily()
{
	unsigned int i, queueFamilyCount;
	const VkPhysicalDevice& gpu = vkInfo.gpu;

	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, NULL);
	AppManager::appAssert(queueFamilyCount >= 1, "there is no queue family found in this gpu.");

	std::vector<VkQueueFamilyProperties> queueFamilyProps(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueFamilyProps.data());

	for (i = 0; i < queueFamilyCount; i++)
	{

		if (queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			vkInfo.graQueueFamilyIndex = i;
		}
	}
	Log::Instance()->log("print gpu info end.");
}

void VulkanLoader::createSurface(GLFWwindow* glfwWin)
{
	VkResult result = glfwCreateWindowSurface(vkInfo.instance, glfwWin, NULL, &vkInfo.surface);

	AppManager::appAssert(result == VK_SUCCESS, "failed to create window surface.");
	unsigned int i = 0;

	std::vector<VkBool32> supported(vkInfo.queueFamilyCount);

	for (i = 0; i < vkInfo.queueFamilyCount; i++) 
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(vkInfo.gpus[vkInfo.gpuIndex], i, vkInfo.surface, &supported[i]);
	}

	bool sameFamily;
	for (i = 0; i < vkInfo.queueFamilyCount; i++)
	{//find a queue family that supports present and graphics mode.
		if (vkInfo.queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && supported[i])
		{
			vkInfo.graQueueFamilyIndex = i;
			vkInfo.preQueueFamilyIndex = i;
			sameFamily = true;
			break;
		}
	}

	if (!sameFamily)
	{//if not found, find one only supports present mode.
		for (i = 0; i < vkInfo.queueFamilyCount; i++)
		{
			VkBool32 supported = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(vkInfo.gpus[vkInfo.gpuIndex], i, vkInfo.surface, &supported);
			if (supported)
			{
				vkInfo.preQueueFamilyIndex = i;
				break;
			}

		}
	}

	
}

void VulkanLoader::createSwapChain()
{
	VkSurfaceCapabilitiesKHR cap;
	VkResult result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkInfo.gpus[vkInfo.gpuIndex], vkInfo.surface, &cap);
	AppManager::appAssert(result == VK_SUCCESS, "cann't acquire surface capabilities in current physical device.");

	VkFormat format;
	uint32_t formatCount;
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(vkInfo.gpus[0], vkInfo.surface, &formatCount, NULL);
	AppManager::appAssert(result == VK_SUCCESS, "something badly happened when acquiring count of surface formats.");
	VkSurfaceFormatKHR *surfFormats = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));
	result = vkGetPhysicalDeviceSurfaceFormatsKHR(vkInfo.gpus[0], vkInfo.surface, &formatCount, surfFormats);
	AppManager::appAssert(result == VK_SUCCESS, "something badly happened when acquiring surface formats.");
	if (formatCount == 1 && surfFormats[0].format == VK_FORMAT_UNDEFINED) {
		format = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else {
		AppManager::appAssert(formatCount >= 1, "there exits no format in current physical device");
		format = surfFormats[0].format;
	}
	free(surfFormats);

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
}

void VulkanLoader::createLogicalDevice()
{
	float queuePriorityes[] = { 0.0f };
	VkDeviceQueueCreateInfo qcInfo = {};
	qcInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	qcInfo.queueCount = 1;
	qcInfo.pQueuePriorities = queuePriorityes;
	qcInfo.queueFamilyIndex = vkInfo.graQueueFamilyIndex;

	VkDeviceCreateInfo devInfo = {};
	devInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	devInfo.queueCreateInfoCount = 1;
	devInfo.pQueueCreateInfos = &qcInfo;

	VkResult result = vkCreateDevice(vkInfo.gpu, &devInfo, NULL, &vkInfo.device);
	AppManager::appAssert(result == VK_SUCCESS, "something bad happened when creating device.");
}

void VulkanLoader::createCommandPool()
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
}

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
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		// Message is important enough to show
		std::cout << "validation layer: " << pCallbackData->pMessage << std::endl;
	}

	return VK_FALSE;
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
	int score;

	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

	if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
		score += 10;

	score += deviceProperties.limits.maxImageDimension2D;

	if (!deviceFeatures.geometryShader) {
		return 0;
	}

	return score;
}






