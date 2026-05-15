#pragma once

#include <vector>

namespace App {
	struct Config {
		const uint32_t WIDTH = 800;
		const uint32_t HEIGHT = 600;

#ifdef NDEBUG
		constexpr bool enableValidationLayers = false;
#else
		constexpr bool enableValidationLayers = true;
#endif

	};
}