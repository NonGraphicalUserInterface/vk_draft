add_library(Window
	# Source
	src/Window.cpp

	# Header
	include/Window/Window.hpp
)

target_include_directories(Window PUBLIC
	include
)

target_include_directories(Window PRIVATE
	include/Window
)

target_link_libraries(Window PUBLIC
	glfw
	Vulkan::Vulkan
	Config
)