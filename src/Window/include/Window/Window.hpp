#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <Config/Config.hpp>

namespace Window {
	class Window {
	public:
		Window(const Config::Config& config);
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;

		// Methods
		inline GLFWwindow* getHandle() const { return window_; }
		inline bool shouldClose() const { return glfwWindowShouldClose(window_); }
		inline void pollEvents() const { glfwPollEvents(); }
	private:
		GLFWwindow* window_;
	};
}