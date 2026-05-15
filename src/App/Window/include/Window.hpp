#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace App {
	class Window {
	public:
		Window();
		~Window();

		Window(const Window&) = delete;
		Window& operator=(const Window&) = delete;
		Window(Window&&) = delete;
		Window& operator=(Window&&) = delete;

		// Methods
		inline bool shouldClose() const { return glfwWindowShouldClose(window_); }
		inline void pollEvents() const { glfwPollEvents(); }
	private:
		GLFWwindow* window_;
	};
}