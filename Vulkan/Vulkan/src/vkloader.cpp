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
	createSuface(glfwWin);
	enumPhysicalDevice();
	for (unsigned int i = 0; i < vkInfo.gpus.size(); i++)
	{
		checkQueuiFamily(i);
	}
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

void VulkanLoader::createSuface(GLFWwindow* glfwWin)
{
	VkResult result = glfwCreateWindowSurface(vkInfo.instance, glfwWin, NULL, &vkInfo.surface);
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
	unsigned int queueFamilyCount, i;
	const VkPhysicalDevice& gpu = vkInfo.gpus[gpuIndex];
	std::vector<VkQueueFamilyProperties> queueProps;

	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, NULL);
	AppManager::appAssert(queueFamilyCount >= 1, "there is no queue family found in this gpu.");

	queueProps.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queueProps.data());

	Log::Instance()->log("print gpu info start.");
	Log::Instance()->log("gpu:" + std::to_string(gpuIndex) + "\tfamily count:" + std::to_string(queueFamilyCount));
	for (i = 0; i < queueFamilyCount; i++) 
	{
		std::string msg = "family:" + std::to_string(i) + "\t";
		msg += "flag:" + std::to_string(queueProps[i].queueFlags);
		msg += "\tcount:" + std::to_string(queueProps[i].queueCount);
		msg += "\ttimestampValidBits:" + std::to_string(queueProps[i].timestampValidBits);
		msg += "\tminImageTransferGranularity:" + 
			std::to_string(queueProps[i].minImageTransferGranularity.width) + ','
			+ std::to_string(queueProps[i].minImageTransferGranularity.height) + ','
			+ std::to_string(queueProps[i].minImageTransferGranularity.depth);
		Log::Instance()->log(msg);
		if (queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			vkInfo.gpuIndex = gpuIndex;
			vkInfo.queueFamilyIndex = i;
		}
	}
	Log::Instance()->log("print gpu info end.");
}

void VulkanLoader::createLogicalDevice()
{
	float queuePriorityes[] = { 0.0f };
	VkDeviceQueueCreateInfo qcInfo = {};
	qcInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	qcInfo.queueCount = 1;
	qcInfo.pQueuePriorities = queuePriorityes;
	qcInfo.queueFamilyIndex = vkInfo.queueFamilyIndex;

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
	cpInfo.queueFamilyIndex = vkInfo.queueFamilyIndex;

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

