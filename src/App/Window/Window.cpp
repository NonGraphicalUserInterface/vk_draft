#include "Window.hpp"

void Window::init() {
	glfwInit();

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window_ = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan Window", nullptr, nullptr);
}

void Window::cleanup() {
	glfwDestroyWindow(window_);
	glfwTerminate();
}