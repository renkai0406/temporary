#include "pch.h"
#include "shadermng.h"


ShaderManager::ShaderManager()
{
}


ShaderManager::~ShaderManager()
{
}

void ShaderManager::init(const VkDevice& device, const std::string * files, int count)
{
	this->device = device;
	
	auto vertModule = createShaderModule(readFile(files[0]));
	auto geomModule = createShaderModule(readFile(files[2]));

	createPippline(vertModule, geomModule);



	vkDestroyShaderModule(device, vertModule, nullptr);
	vkDestroyShaderModule(device, geomModule, nullptr);
}

std::vector<char> ShaderManager::readFile(const std::string & fname)
{
	if (fname.size() < 1)
		return std::vector<char>();
	std::ifstream file(fname, std::ios::ate | std::ios::binary);

	AppManager::appAssert(file.is_open(), "failed to open file:" + fname);

	unsigned int size = (unsigned int)file.tellg();
	std::vector<char> buffer(size);

	file.seekg(0);
	file.read(buffer.data(), size);

	file.close();

	return buffer;

}

VkShaderModule ShaderManager::createShaderModule(const std::vector<char>& codes)
{
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = codes.size();
	createInfo.pCode = reinterpret_cast<const unsigned int*>(codes.data());

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule);
	AppManager::appAssert(result != VK_SUCCESS, "failed to create shader module!");

	return shaderModule;
}

void ShaderManager::createPippline(VkShaderModule & vert, VkShaderModule & geom)
{
	VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vert;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = geom;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };
}
