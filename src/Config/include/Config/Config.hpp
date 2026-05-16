#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.h>

namespace Config {
	struct Config {
		// Window
		const uint32_t width;
		const uint32_t height;

		std::string title;

		// Vulkan
		const std::vector<const char*> validationLayers = {
			"VK_LAYER_KHRONOS_validation"
		};

#ifdef NDEBUG
		const bool enableValidationLayers = false;
#else
		const bool enableValidationLayers = true;
#endif
		// Debug messenger
		struct MessageInfo {
			// Finish later
		};

		VkDebugUtilsMessageSeverityFlagsEXT messageSeverity =
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
		;

		VkDebugUtilsMessageTypeFlagsEXT messageType =
			VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
			VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
		;
	};
}