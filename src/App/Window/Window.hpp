#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

class Window {
public:
	Window() = default;
	~Window() = default;

	Window(const Window&) = delete;
	Window& operator=(const Window&) = delete;
	Window(Window&&) = delete;
	Window& operator=(Window&&) = delete;

	// Methods
	void init();
	inline bool shouldClose() const { return glfwWindowShouldClose(window_); }
	inline void pollEvents() const { glfwPollEvents(); }
	void cleanup();
private:
	GLFWwindow* window_;
};