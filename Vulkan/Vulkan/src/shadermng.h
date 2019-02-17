#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include <vector>
#include<fstream>

#include <vulkan\vulkan.h>

#include "appmng.h"

class ShaderManager
{
public:
	ShaderManager();
	~ShaderManager();

	void init(const VkDevice& device, const std::string* files, int count, const VkExtent2D& extent);
	void clearup();

private:
	VkPipelineLayout pipelineLayout;
	VkDevice device;
	VkExtent2D extent;
	static std::vector<char> readFile(const std::string& fname);
	VkShaderModule createShaderModule(const std::vector<char>& codes);
	void createPippline(VkShaderModule& vert, VkShaderModule& geom);
	
};

#endif

