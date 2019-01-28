#ifndef VKLOADER_H
#define VKLOADER_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "log.h"
#include "appmng.h"

#define APP_NAME "vulkan learning"

const unsigned int GPU_NEEDED_COUNT = 1;

struct VkInfo
{
	VkInstance instance;
	VkDevice device;
	std::vector<VkPhysicalDevice> gpus;
	unsigned int gpuIndex;
	unsigned int queueFamilyIndex;
	VkCommandPool cpool;
	VkCommandBuffer cbuffer;
};

class VulkanLoader
{
public:
	VulkanLoader();
	~VulkanLoader();
	static void init();
	static void destory();
private:
	static void createInstance();
	static void enumPhysicalDevice();
	static void checkQueuiFamily(unsigned int gpuIndex);
	static void createLogicalDevice();
	static void createCommandPool();
	static void createCommandBuffer();
	static void createSuface();

private:
	static VkInfo vkInfo;

};

#endif //VKLOADER_H!

