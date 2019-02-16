#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include <vector>
#include<fstream>

#include <vulkan\vulkan.h>

#include "appmng.h"

#define VULKAN_BIN_DIR "D:/VulkanSDK/1.1.85.0/Bin32"

class ShaderManager
{
public:
	ShaderManager();
	~ShaderManager();

	void init(const VkDevice& device, const std::string* files, int count);

private:
	VkDevice device;
	static std::vector<char> readFile(const std::string& fname);
	VkShaderModule createShaderModule(const std::vector<char>& codes);
	void createPippline(VkShaderModule& vert, VkShaderModule& geom);
	
};

#endif

