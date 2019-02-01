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
	std::vector<VkPhysicalDevice> gpus;
	unsigned int gpuIndex;
	VkDevice device;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	unsigned int width, height;
	//unsigned int queueFamilyIndex;
	unsigned int graQueueFamilyIndex, preQueueFamilyIndex;
	unsigned int queueFamilyCount;
	std::vector<VkQueueFamilyProperties> queueFamilyProps;
	VkCommandPool cpool;
	VkCommandBuffer cbuffer;
	bool enableLayers;
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
	 std::vector<const char*> getRequiredExtensions();
	

private:
	 VkInfo vkInfo;

};

#endif //VKLOADER_H!

