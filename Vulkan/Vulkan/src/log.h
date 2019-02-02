#ifndef LOG_H
#define LOG_H

#include <iostream>
#include<string>
#include<Windows.h>

class Log
{
public:
	static Log* const Instance();
	void init();
	void log(std::string info);
	void error(std::string info);
private:
	static Log* instance;
	Log();
	~Log();
};

class FileLog : public Log {
public:
private:
	const int BUFFER_SIZE = 1024;
	
};

#endif //LOG_H!



