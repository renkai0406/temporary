#ifndef UI_H
#define UI_H

#include <GLFW/glfw3.h>
#include "log.h"

void framebuffer_resize_callback(GLFWwindow*, int, int);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double x, double y);
void scroll_callback(GLFWwindow* window, double x, double y);

class UI
{
public:
	void init(const std::string& title);
	void mainloop();
	void clearup();
	bool shouldCloseWin();
	GLFWwindow* getGlfwWindow();
	int win_width = 1024, win_height = 768;
private:
	GLFWwindow * window;
	
};

#endif

