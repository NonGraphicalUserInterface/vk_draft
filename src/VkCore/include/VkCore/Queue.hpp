#pragma once

#include <optional>
#include <cstdint>

#include <vulkan/vulkan.h>

namespace VkCore {
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;

		inline bool isComplete() {
			return graphicsFamily.has_value();
		}
	};

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
}