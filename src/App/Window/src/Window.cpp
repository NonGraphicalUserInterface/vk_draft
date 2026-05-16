#include "Window.hpp"

namespace App {
	Window::Window(const Config::Config& config) {
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window_ = glfwCreateWindow(config.width, config.height, config.title.data(), nullptr, nullptr);
	}

	Window::~Window() {
		glfwDestroyWindow(window_);
		glfwTerminate();
	}
}