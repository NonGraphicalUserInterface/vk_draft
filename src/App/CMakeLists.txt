add_library(App
	src/App.cpp

	include/App/App.hpp
)

target_include_directories(App PUBLIC
	include
)

target_include_directories(App PRIVATE
	include/App
)

target_link_libraries(App PUBLIC
	Window
	VkCore
)