#include "pch.h"
#include "ui.h"
#include "appmng.h"

void UI::init(const std::string& title)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window = glfwCreateWindow(win_width, win_height, title.c_str(), NULL, NULL);

	AppManager::appAssert(window != NULL, "failed to create glfw window.");

	glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwMakeContextCurrent(window);
	
}

void UI::mainloop()
{
		glfwPollEvents();
}

void UI::clearup()
{
	glfwDestroyWindow(window);
	glfwTerminate();
}

bool UI::shouldCloseWin()
{
	return glfwWindowShouldClose(window);
}

GLFWwindow * UI::getGlfwWindow()
{
	return window;
}

void framebuffer_resize_callback(GLFWwindow *, int, int)
{
}

void key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
{
}

void mouse_button_callback(GLFWwindow * window, int button, int action, int mods)
{
}

void cursor_position_callback(GLFWwindow * window, double x, double y)
{
}

void scroll_callback(GLFWwindow * window, double x, double y)
{
}

