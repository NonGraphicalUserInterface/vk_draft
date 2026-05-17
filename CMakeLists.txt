cmake_minimum_required (VERSION 3.22)

project (vk_draft)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Vulkan REQUIRED)

# Include sub-projects.
add_subdirectory(third_party)
add_subdirectory(src/Config)
add_subdirectory(src/Window)
add_subdirectory(src/VkCore)
add_subdirectory(src/App)

add_executable (vk_draft
	"src/main.cpp"
)

target_link_libraries(vk_draft PRIVATE
	App
)