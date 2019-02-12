#ifndef VKLOADER_H
#define VKLOADER_H

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <map>
#include <set>
#include "log.h"
#include "appmng.h"

const unsigned int GPU_NEEDED_COUNT = 1;

struct VkInfo
{
	VkInstance instance;
	//the layers we needed.
	const std::vector<const char*> layers = {
	"VK_LAYER_LUNARG_standard_validation"
	};
	const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};
	VkDebugUtilsMessengerEXT debugMessenger;
	VkPhysicalDevice gpu;
	VkDevice device;
	VkSurfaceKHR surface;
	VkSwapchainKHR swapchain;
	unsigned int width, height;
	std::vector<VkImage> swapImages;
	VkExtent2D scExtent;
	VkFormat scFormat;
	//unsigned int queueFamilyIndex;
	unsigned int graQueueFamilyIndex, preQueueFamilyIndex;
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
	 void createSurface(GLFWwindow* glfwWin);
	 void pickPhysicalDevice();
	 void checkQueueFamily();
	 void createLogicalDevice();
	 void createSwapChain();
	 //void createCommandPool();
	 //void createCommandBuffer();
	 bool checkLayersSupport();
	 bool checkExtensionsSupport(const std::vector<const char*>& needs);
	 static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		 VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		 VkDebugUtilsMessageTypeFlagsEXT messageType,
		 const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		 void* pUserData);
	 VkResult CreateDebugUtilsMessengerEXT(const VkInstance &instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	 void DestroyDebugUtilsMessengerEXT(const VkInstance &instance, VkDebugUtilsMessengerEXT &debugMessenger, const VkAllocationCallbacks* pAllocator);
	 int rateDeviceSuitability(VkPhysicalDevice& device);
	
	

private:
	 VkInfo vkInfo;

};

#endif //VKLOADER_H!


