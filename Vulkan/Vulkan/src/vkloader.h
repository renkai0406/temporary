#ifndef VKLOADER_H
#define VKLOADER_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "log.h"
#include "appmng.h"

const unsigned int GPU_NEEDED_COUNT = 1;

struct VkInfo
{
	VkInstance instance;
	VkSurfaceKHR surface;
	VkDevice device;
	std::vector<VkPhysicalDevice> gpus;
	std::vector<VkQueueFamilyProperties> queueFamilyProps;
	unsigned int gpuIndex;
	//unsigned int queueFamilyIndex;
	unsigned int graQueueFamilyIndex, preQueueFamilyIndex;
	unsigned int queueFamilyCount;
	VkCommandPool cpool;
	VkCommandBuffer cbuffer;
};

class VulkanLoader
{
public:
	VulkanLoader();
	~VulkanLoader();
	 void init(const std::string& title, GLFWwindow* glfwWin);
	 void destory();
private:
	 void createInstance(const std::string& title);
	 void enumPhysicalDevice();
	 void checkQueuiFamily(unsigned int gpuIndex);
	 void createSurface(GLFWwindow* glfwWin);
	 void createSwapChain();
	 void createLogicalDevice();
	 void createCommandPool();
	 void createCommandBuffer();
	

private:
	 VkInfo vkInfo;

};

#endif //VKLOADER_H!

