// main.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "pch.h"
#include "vkloader.h"

void init();

int main()
{
	init();
	
	Log::Instance()->log("the program has done,press any key to continue...");
	std::getchar();
	return 0;
	//VulkanLoader::destory();
}

void init() {
	Log::Instance()->init();
	Log::Instance()->log("Hello Vulkan!\n");
	VulkanLoader::init();
}