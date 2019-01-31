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
	enumPhysicalDevice();
	vkInfo.gpuIndex = 0;
	checkQueuiFamily(vkInfo.gpuIndex);
	createSurface(glfwWin);
	createSwapChain();
	createLogicalDevice();
	createCommandPool();
	createCommandBuffer();
}

void VulkanLoader::destory()
{
	vkFreeCommandBuffers(vkInfo.device, vkInfo.cpool, 1, &vkInfo.cbuffer);
	
	vkDestroyCommandPool(vkInfo.device, vkInfo.cpool, NULL);

	vkDeviceWaitIdle(vkInfo.device);
	vkDestroyDevice(vkInfo.device, NULL);

	vkDestroySurfaceKHR(vkInfo.instance, vkInfo.surface, nullptr);

	vkDestroyInstance(vkInfo.instance, NULL);
}

void VulkanLoader::createInstance(const std::string& title)
{
	VkApplicationInfo appInfo = {};
	appInfo.pApplicationName = title.c_str();
	appInfo.applicationVersion = 1;
	appInfo.pEngineName = title.c_str();
	appInfo.engineVersion = 1;
	appInfo.apiVersion = VK_API_VERSION_1_0;


	VkInstanceCreateInfo icInfo = {};
	icInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	icInfo.pApplicationInfo = &appInfo;

	VkResult result;
	if ((result = vkCreateInstance(&icInfo, NULL, &vkInfo.instance)) == VK_ERROR_INCOMPATIBLE_DRIVER) {
		Log::Instance()->log("cannot find a compatible Vulkan ICD");
		AppManager::appExit();
	}
	else if (result) {
		Log::Instance()->log("unknown error:" + result);
		AppManager::appExit();
	}

}

void VulkanLoader::enumPhysicalDevice()
{
	unsigned int gpuCount;
	// Get the number of devices (GPUs) available.
	VkResult res = vkEnumeratePhysicalDevices(vkInfo.instance, &gpuCount, NULL);
	AppManager::appAssert(gpuCount >= GPU_NEEDED_COUNT, "the number of gpu is less than needed.");
	// Allocate space and get the list of devices.
	vkInfo.gpus.resize(gpuCount);
	res = vkEnumeratePhysicalDevices(vkInfo.instance, &gpuCount, vkInfo.gpus.data());
	AppManager::appAssert(!res, "something bad happened when enumerating physic devices.");
}

void VulkanLoader::checkQueuiFamily(unsigned int gpuIndex)
{
	unsigned int i;
	const VkPhysicalDevice& gpu = vkInfo.gpus[gpuIndex];

	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &vkInfo.queueFamilyCount, NULL);
	AppManager::appAssert(vkInfo.queueFamilyCount >= 1, "there is no queue family found in this gpu.");

	vkInfo.queueFamilyProps.resize(vkInfo.queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &vkInfo.queueFamilyCount, vkInfo.queueFamilyProps.data());

	Log::Instance()->log("print gpu info start.");
	Log::Instance()->log("gpu:" + std::to_string(gpuIndex) + "\tfamily count:" + std::to_string(vkInfo.queueFamilyCount));
	for (i = 0; i < vkInfo.queueFamilyCount; i++)
	{
		std::string msg = "family:" + std::to_string(i) + "\t";
		msg += "flag:" + std::to_string(vkInfo.queueFamilyProps[i].queueFlags);
		msg += "\tcount:" + std::to_string(vkInfo.queueFamilyProps[i].queueCount);
		msg += "\ttimestampValidBits:" + std::to_string(vkInfo.queueFamilyProps[i].timestampValidBits);
		msg += "\tminImageTransferGranularity:" + 
			std::to_string(vkInfo.queueFamilyProps[i].minImageTransferGranularity.width) + ','
			+ std::to_string(vkInfo.queueFamilyProps[i].minImageTransferGranularity.height) + ','
			+ std::to_string(vkInfo.queueFamilyProps[i].minImageTransferGranularity.depth);
		Log::Instance()->log(msg);
		if (vkInfo.queueFamilyProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			vkInfo.gpuIndex = gpuIndex;
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

	sccInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	
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

	result = vkCreateSwapchainKHR(vkInfo.device, &sccInfo, NULL, &vkInfo.swapchain);
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

	VkResult result = vkCreateDevice(vkInfo.gpus[vkInfo.gpuIndex], &devInfo, NULL, &vkInfo.device);
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

