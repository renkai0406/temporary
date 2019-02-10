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
	//the layers we needed.
	const std::vector<const char*> layers = {
	"VK_LAYER_LUNARG_standard_validation"};
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice gpu;
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
#ifdef NDEBUG
	const bool layersEnabled = false;
#else
	const bool layersEnabled = true;
#endif
};

class VulkanLoader
{
public:
	VulkanLoader();
	~VulkanLoader();
	 void init(const std::string& title, GLFWwindow* glfwWin);
	 void clearup();
private:
	 void createInstance(const std::string& title);
	 void setupDebugMessenger();
	 void pickPhysicalDevice();
	 void checkQueuiFamily(unsigned int gpuIndex);
	 void createSurface(GLFWwindow* glfwWin);
	 void createSwapChain();
	 void createLogicalDevice();
	 void createCommandPool();
	 void createCommandBuffer();
	 bool checkLayersSupport();
	 bool checkExtensionsSupport(const std::vector<const char*>& needs);
	 static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		 VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		 VkDebugUtilsMessageTypeFlagsEXT messageType,
		 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		 void* pUserData);
	 VkResult CreateDebugUtilsMessengerEXT(const VkInstance &instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	 void DestroyDebugUtilsMessengerEXT(const VkInstance &instance, VkDebugUtilsMessengerEXT &debugMessenger, const VkAllocationCallbacks* pAllocator);
	 bool isDeviceSuitable(VkPhysicalDevice& device);
	
	

private:
	 VkInfo vkInfo;

};

#endif //VKLOADER_H!


