#include "Window.hpp"

#include <stdexcept>

namespace Window {
	Window::Window(const Config::Config& config) {
		if (!glfwInit()) {
			throw std::runtime_error("Failed to initialise GLFW!");
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		window_ = glfwCreateWindow(config.width, config.height, config.title.data(), nullptr, nullptr);
		if (window_ == nullptr) {
			throw std::runtime_error("Failed to create GLFW window!");
		}
	}

	Window::~Window() {
		glfwDestroyWindow(window_);
		glfwTerminate();
	}
}