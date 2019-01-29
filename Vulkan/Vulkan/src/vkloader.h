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
	 void init(const std::string& title, GLFWwindow* glfwWin);
	 void destory();
private:
	 void createInstance(const std::string& title);
	 void createSuface(GLFWwindow* glfwWin);
	 void enumPhysicalDevice();
	 void checkQueuiFamily(unsigned int gpuIndex);
	 void createLogicalDevice();
	 void createCommandPool();
	 void createCommandBuffer();
	

private:
	 VkInfo vkInfo;

};

#endif //VKLOADER_H!

