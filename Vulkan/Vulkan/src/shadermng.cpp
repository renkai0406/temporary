#include "pch.h"
#include "shadermng.h"


ShaderManager::ShaderManager()
{
}


ShaderManager::~ShaderManager()
{
}

void ShaderManager::init(const std::string * files, int count)
{
	unsigned int i = 0;
	for (i = 0; i < count; i++)
	{
		auto codes = readFile(files[i]);
	}
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
