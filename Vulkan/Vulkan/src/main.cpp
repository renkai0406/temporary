// main.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "pch.h"
#include "maincfg.h"
#include "vkloader.h"
#include "ui.h"

VulkanLoader vkloader;
UI window;

void init();
void run();
void clearup();

int main()
{
	try
	{
		init();
		run();
		clearup();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

void init() {
	Log::Instance()->init();
	Log::Instance()->log("Hello Vulkan!\n");
	window.init(title);
	vkloader.init(title, window.getGlfwWindow());
}

void run()
{
	while (!window.shouldCloseWin())
	{
		window.mainloop();
	}
	
}

void clearup()
{
	
	vkloader.clearup();
	window.clearup();
	Log::Instance()->log("the program has done,press any key to continue...");
	std::getchar();
}
