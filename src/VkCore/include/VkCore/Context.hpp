#pragma once

#include <Config/Config.hpp>

#include <vulkan/vulkan.h>


namespace VkCore {
	class Context {
	public:
		Context(const Config::Config* config);
		~Context();

		// No copying
		Context(const Context&) = delete;
		Context& operator=(const Context&) = delete;

		// Can move
		Context(Context&& other) noexcept;
		Context& operator=(Context&& other) noexcept;
	private:
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


		/* GENERAL */
		void cleanup();
	};
}