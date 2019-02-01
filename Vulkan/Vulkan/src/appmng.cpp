#include "pch.h"
#include "appmng.h"

AppManager::AppManager()
{
}


AppManager::~AppManager()
{
}

void AppManager::appExit()
{
	Log::Instance()->log("the application must be terminated! Please enter any key to continue...;");
	getchar();
	exit(-1);
}

void AppManager::appAssert(int exp, std::string msg)
{
	if (exp <= 0) {
		//throw std::runtime_error(msg);
		Log::Instance()->error(msg);
		appExit();
	}
}
