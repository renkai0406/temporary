#include "pch.h"
#include "ui.h"
#include "appmng.h"

void UI::init(const std::string& title)
{
	glfwInit();
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	window = glfwCreateWindow(win_width, win_height, title.c_str(), NULL, NULL);

	if (window == NULL)
	{
		Log::Instance()->error("failed to create glfw window");
		AppManager::appExit();
	}

	glfwSetFramebufferSizeCallback(window, framebuffer_resize_callback);
	glfwSetKeyCallback(window, key_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwMakeContextCurrent(window);
	
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

