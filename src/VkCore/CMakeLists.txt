add_library(VkCore
	# Source
	src/Instance.cpp

	# Header
	include/VkCore/Instance.hpp
)

target_include_directories(VkCore PUBLIC
	include
)

target_link_libraries(VkCore PUBLIC
	Vulkan::Vulkan
	glfw
)