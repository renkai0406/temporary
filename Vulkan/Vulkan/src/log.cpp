#include "pch.h"
#include "log.h"

Log Log::instance;

Log::Log()
{
}


Log::~Log()
{
}

Log& Log::Instance()
{
	return instance;
}

void Log::init()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE);
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
	
}

void Log::log(std::string info)
{
	std::cout << info << std::endl;
}

void Log::error(std::string info)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
	log("error:" + info);
	init();
}
