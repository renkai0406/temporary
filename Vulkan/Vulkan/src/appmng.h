#ifndef APPMANAGER_H
#define APPMANAGER_H

#include <stdlib.h>
#include <assert.h>
#include "log.h"

class AppManager
{
public:
	AppManager();
	~AppManager();
	static void appExit();
	static void appAssert(int exp, std::string msg);
	static void appError(std::string msg);
};

#endif

