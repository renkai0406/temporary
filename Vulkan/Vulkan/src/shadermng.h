#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include <vector>
#include<fstream>

#include "appmng.h"

class ShaderManager
{
public:
	ShaderManager();
	~ShaderManager();

	void init(const std::string* files, int count);

private:
	static std::vector<char> readFile(const std::string& fname);
};

#endif

