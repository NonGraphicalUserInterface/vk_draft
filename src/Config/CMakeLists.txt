add_library(Config INTERFACE)

target_include_directories(Config INTERFACE
	include
)

target_link_libraries(Config INTERFACE
	Vulkan::Vulkan
)