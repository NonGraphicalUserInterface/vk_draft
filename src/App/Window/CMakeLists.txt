add_library(Window
	src/Window.cpp

	include/Window.hpp
)

target_include_directories(Window PUBLIC
	include
)

target_link_libraries(Window PUBLIC
	glfw
	Vulkan::Vulkan
)