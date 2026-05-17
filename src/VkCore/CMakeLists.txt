add_library(VkCore
	# Source
	src/Context.cpp
	src/Queue.cpp

	# Header
	include/VkCore/Context.hpp
	include/VkCore/Queue.hpp
 )

target_include_directories(VkCore PUBLIC
	include
)

target_include_directories(VkCore PRIVATE include/VkCore)

target_link_libraries(VkCore PUBLIC
	Vulkan::Vulkan
	glfw
	Config
	Window
)