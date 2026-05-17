#pragma once

#include <Config/Config.hpp>
#include <Window/Window.hpp>

#include "Swapchain.hpp"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>


namespace VkCore {
	class Context {
	public:
		Context(const Window::Window* window, const Config::Config* config);
		~Context();

		// No copying
		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;

		// Can move
		Context(Context&& other) noexcept;
		Context& operator=(Context&& other) noexcept;
	private:
		const Window::Window* window_;
		const Config::Config* config_;

		/* INSTANCE */
		VkInstance instance_ = VK_NULL_HANDLE;
		void createInstance();
		bool checkValidationLayerSupport();
		std::vector<const char*> getRequiredExtensions();
		void destroyInstance();

		/* DEBUG MESSENGER */
		VkDebugUtilsMessengerEXT debugMessenger_ = VK_NULL_HANDLE;
		void createDebugMessenger();
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData
		);

		// Since vkCreateDebugUtilsMessengerEXT and vkDestroyDebugUtilsMessengerEXT are external, we must load manually before using
		VkResult createDebugUtilsMessengerEXT(
			VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
			const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger
		);

		void destroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);
		void destroyDebugMessenger();

		/* PHYSICAL DEVICE */
		VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
		void pickPhysicalDevice();
		bool checkDeviceExtensionSupport(VkPhysicalDevice device);
		bool isDeviceSuitable(VkPhysicalDevice device);

		/* LOGICAL DEVICE */
		VkDevice device_ = VK_NULL_HANDLE;
		VkQueue graphicsQueue_ = VK_NULL_HANDLE;
		VkQueue presentQueue_ = VK_NULL_HANDLE;
		void createDevice();
		void destroyDevice();

		/* WINDOW SURFACE */
		VkSurfaceKHR surface_ = VK_NULL_HANDLE;
		void createSurface();
		void destroySurface();
		
		/* SWAPCHAIN */
		VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
		std::vector<VkImage> swapchainImages_ = {VK_NULL_HANDLE}; // Image: actual data buffer
		std::vector<VkImageView> swapchainImageViews_ = {VK_NULL_HANDLE}; // Image view: metadata -- tells Vulkan how to read/write to Image
		SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
		VkPresentModeKHR chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
		VkExtent2D chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window);
		void createSwapchain();
		void getSwapchainImages();
		void createSwapchainImageViews();
		void setupSwapchainResources();
		void cleanupSwapchainResources();

		/* DEPTH RESOURCES */
		VkImage depthImage_ = VK_NULL_HANDLE;
		VkDeviceMemory depthImageMemory_ = VK_NULL_HANDLE;
		VkImageView depthImageView_ = VK_NULL_HANDLE;
		VkFormat depthFormat_ = VK_FORMAT_UNDEFINED;
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		inline bool hasStencilComponent(VkFormat format) {
			return
				format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
				format == VK_FORMAT_D24_UNORM_S8_UINT ||
				format == VK_FORMAT_D16_UNORM_S8_UINT
			;
		}
		void createDepthResources();

		/* GENERAL */
		void cleanup();
	};
}