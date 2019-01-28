#ifndef LOG_H
#define LOG_H

#include <iostream>
#include<string>
#include<Windows.h>

class Log
{
public:
	static Log& Instance();
	void init();
	void log(std::string info);
	void error(std::string info);
private:
	static Log instance;
	Log();
	~Log();
};

#endif //LOG_H!

