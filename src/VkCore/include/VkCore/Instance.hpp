#pragma once

#define GLFW_INCLUDE_NONE
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <vector>

class Instance {
public:
	Instance();
	~Instance();
private:
	VkInstance instance_ = nullptr;
};