#include "Context.hpp"

#include <stdexcept>
#include <iostream>
#include <cstdint>
#include <string>
#include <set>
#include <limits>
#include <algorithm>

#include "Queue.hpp"

namespace VkCore {
	Context::Context(const Window::Window* window, const Config::Config* config) :
		config_(config),
		window_(window)
	{
		std::cout << "Creating context" << std::endl;
		createInstance();
		createDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createDevice();
	}

	Context::~Context() {
		cleanup();
	}

	// Moving
	Context::Context(Context&& other) noexcept :
		window_				(std::exchange(other.window_,				nullptr)),
		config_				(std::exchange(other.config_,				nullptr)),
		instance_			(std::exchange(other.instance_,				VK_NULL_HANDLE)),
		debugMessenger_		(std::exchange(other.debugMessenger_,		VK_NULL_HANDLE)),
		physicalDevice_		(std::exchange(other.physicalDevice_,		VK_NULL_HANDLE)),
		device_				(std::exchange(other.device_,				VK_NULL_HANDLE)),
		graphicsQueue_		(std::exchange(other.graphicsQueue_,		VK_NULL_HANDLE)),
		presentQueue_		(std::exchange(other.presentQueue_,			VK_NULL_HANDLE)),
		surface_			(std::exchange(other.surface_,				VK_NULL_HANDLE)),
		swapchain_			(std::exchange(other.swapchain_,			VK_NULL_HANDLE)),
		swapchainImages_	(std::move(other.swapchainImages_)),
		swapchainImageViews_(std::move(other.swapchainImageViews_)),
		depthImage_			(std::exchange(other.depthImage_,			VK_NULL_HANDLE)),
		depthImageMemory_	(std::exchange(other.depthImageMemory_,		VK_NULL_HANDLE)),
		depthImageView_		(std::exchange(other.depthImageView_,		VK_NULL_HANDLE))
	{}

	Context& Context::operator=(Context&& other) noexcept {
		if (this == &other) return *this;

		cleanup();

		window_ =				std::exchange(other.window_,				nullptr);
		config_ =				std::exchange(other.config_,				nullptr);
		instance_ =				std::exchange(other.instance_,				VK_NULL_HANDLE);
		debugMessenger_ =		std::exchange(other.debugMessenger_,		VK_NULL_HANDLE);
		physicalDevice_ =		std::exchange(other.physicalDevice_,		VK_NULL_HANDLE);
		device_ =				std::exchange(other.device_,				VK_NULL_HANDLE);
		graphicsQueue_ =		std::exchange(other.graphicsQueue_,			VK_NULL_HANDLE);
		presentQueue_ =			std::exchange(other.presentQueue_,			VK_NULL_HANDLE);
		surface_ =				std::exchange(other.surface_,				VK_NULL_HANDLE);
		swapchain_ =			std::exchange(other.swapchain_, 			VK_NULL_HANDLE);
		swapchainImages_ =		std::move(other.swapchainImages_);
		swapchainImageViews_ =	std::move(other.swapchainImageViews_);
		depthImage_ =			std::exchange(other.depthImage_,			VK_NULL_HANDLE);
		depthImageMemory_ =		std::exchange(other.depthImageMemory_,		VK_NULL_HANDLE);
		depthImageView_ =		std::exchange(other.depthImageView_,		VK_NULL_HANDLE);

		return *this;
	}

	/* INSTANCE */
	void Context::createInstance() {
		// First, check support
		if (config_->enableValidationLayers && !checkValidationLayerSupport()) {
			throw std::runtime_error("Validation layers requested, but not supported!");
		}

		// App info
		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = "Vulkan Draft";
		appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.pEngineName = "No Engine";
		appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		appInfo.apiVersion = VK_API_VERSION_1_4;

		// Instance creation info
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// Print extension support
		uint32_t extensionCount = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> properties(extensionCount);
		vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, properties.data());

		std::cout << "Found extensions:" << std::endl;
		for (const auto& property : properties) {
			std::cout << "\t" << property.extensionName << std::endl;
		}

		// Get extensions (for now, GLFW and, if needed, debug messenger ones)
		std::vector<const char*> extensions = getRequiredExtensions();
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		// Validation layers
		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{}; // This is for the debug messenger used ONLY during instance creation / destruction
		if (config_->enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(config_->validationLayers.size());
			createInfo.ppEnabledLayerNames = config_->validationLayers.data();

			// Populate debugCreateInfo
			debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugCreateInfo.messageSeverity = config_->messageSeverity;
			debugCreateInfo.messageType = config_->messageType;
			//debugCreateInfo.pfnUserCallback = ;
		}
		else {
			createInfo.enabledLayerCount = 0;
			createInfo.ppEnabledLayerNames = nullptr;
		}

		// Create instance
		if (vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create Vulkan instance");
		}
		else {
			std::cout << "Initialised Vulkan instance" << std::endl;
		}
	}

	bool Context::checkValidationLayerSupport() {
		// Get available layers
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		// See if any are true
		for (const char* layerName : config_->validationLayers) {
			bool layerFound = false;
			for (const VkLayerProperties& availableLayer : availableLayers) {
				if (strcmp(availableLayer.layerName, layerName) == 0) {
					layerFound = true;
					break;
				}
			}

			if (!layerFound) return false;
		}

		return true;
	}

	std::vector<const char*> Context::getRequiredExtensions() {
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (config_->enableValidationLayers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		return extensions;
	}

	void Context::destroyInstance() {
		if (instance_ != VK_NULL_HANDLE) {
			vkDestroyInstance(instance_, nullptr);
			instance_ = VK_NULL_HANDLE;
		}
	}

	/* DEBUG MESSENGER */
	void Context::createDebugMessenger() {
		if (!config_->enableValidationLayers) return;

		VkDebugUtilsMessengerCreateInfoEXT createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.messageSeverity = config_->messageSeverity; // Which severity to call callback for
		createInfo.messageType = config_->messageType; // Which types of messages to tell callback about
		createInfo.pfnUserCallback = debugCallback;
		createInfo.pUserData = nullptr;

		if (createDebugUtilsMessengerEXT(instance_, &createInfo, nullptr, &debugMessenger_) != VK_SUCCESS) {
			throw std::runtime_error("Failed to set up debug messenger!");
		}
		else {
			std::cout << "Initialised debug messenger" << std::endl;
		}
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL Context::debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
	) {
		if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
			std::cerr << "Validation Layer: " << pCallbackData->pMessage << std::endl;
		}

		return VK_FALSE;
	}

	VkResult Context::createDebugUtilsMessengerEXT(
		VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger
	) {
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr) {
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		}
		else {
			return VK_ERROR_EXTENSION_NOT_PRESENT;
		}
	}

	void Context::destroyDebugUtilsMessengerEXT(
		VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
		const VkAllocationCallbacks* pAllocator
	) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr) {
			func(instance, debugMessenger, pAllocator);
		}
	}

	void Context::destroyDebugMessenger() {
		if (debugMessenger_ != VK_NULL_HANDLE) {
			destroyDebugUtilsMessengerEXT(instance_, debugMessenger_, nullptr);
			debugMessenger_ = VK_NULL_HANDLE;
		}
	}

	/* PHYSICAL DEVICE */
	void Context::pickPhysicalDevice() {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
		if (deviceCount == 0) {
			throw std::runtime_error("Failed to find GPUs with Vulkan support!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

		for (const VkPhysicalDevice& device : devices) {
			if (isDeviceSuitable(device)) {
				physicalDevice_ = device;
				break;
			}
		}

		if (physicalDevice_ == VK_NULL_HANDLE) {
			throw std::runtime_error("Failed to find a suitable GPU!");
		}
	}

	bool Context::checkDeviceExtensionSupport(VkPhysicalDevice device) {
		uint32_t extensionCount = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

		std::set<std::string> required(config_->deviceExtensions.begin(), config_->deviceExtensions.end());
		for (const VkExtensionProperties& extension : availableExtensions) {
			required.erase(extension.extensionName);
		}

		return required.empty();
	}

	bool Context::isDeviceSuitable(VkPhysicalDevice device) {
		// We must check for supported queues, swapchain support and swapchain suitability
		bool queuesSupported = findQueueFamilies(device, surface_).isComplete();
		bool extensionsSupported = checkDeviceExtensionSupport(device);
		SwapchainSupportDetails swapchainSupport = querySwapchainSupport(device);
		bool swapchainSuitable = !swapchainSupport.formats.empty() && !swapchainSupport.presentModes.empty();
		return queuesSupported && extensionsSupported && swapchainSuitable;
	}

	/* LOGICAL DEVICE */
	void Context::createDevice() {
		// Logical device needs to know what queue families it can use
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice_, surface_);

		// Create both graphics and present queues
		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
		
		float queuePriority = 1.0f;

		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo{};
			queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfo.queueFamilyIndex = queueFamily;
			queueCreateInfo.queueCount = 1;
			queueCreateInfo.pQueuePriorities = &queuePriority;
			queueCreateInfos.push_back(queueCreateInfo);
		}

		// We will come back to device features later
		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = &deviceFeatures;
		createInfo.enabledExtensionCount = static_cast<uint32_t>(config_->deviceExtensions.size());
		createInfo.ppEnabledExtensionNames = config_->deviceExtensions.data();

		// Previous implementations distinguished between instance- and device-specific validation layers, so this is for compatibility
		if (config_->enableValidationLayers) {
			createInfo.enabledLayerCount = static_cast<uint32_t>(config_->validationLayers.size());
			createInfo.ppEnabledLayerNames = config_->validationLayers.data();
		}
		else {
			createInfo.enabledLayerCount = 0;
		}

		// Create device
		if (vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create logical device!");
		}

		// Queue
		vkGetDeviceQueue(device_, indices.graphicsFamily.value(), 0, &graphicsQueue_);
		vkGetDeviceQueue(device_, indices.presentFamily.value(), 0, &presentQueue_);
	}

	void Context::destroyDevice() {
		if (device_ != VK_NULL_HANDLE) {
			vkDestroyDevice(device_, nullptr);
			device_ = VK_NULL_HANDLE;
		}
	}

	/* WINDOW SURFACE */
	void Context::createSurface() {
		if (glfwCreateWindowSurface(instance_, window_->getHandle(), nullptr, &surface_) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create window surface!");
		}
	}

	void Context::destroySurface() {
		if (surface_ != VK_NULL_HANDLE) {
			vkDestroySurfaceKHR(instance_, surface_, nullptr);
			surface_ = VK_NULL_HANDLE;
		}
	}

	/* SWAPCHAIN */
	// Just having *a* swapchain is insufficient -- it might not be compatible with the window surface.
	// So, this helps us query and process the details of what is available
	SwapchainSupportDetails Context::querySwapchainSupport(VkPhysicalDevice device) {
		SwapchainSupportDetails details;

		// Basic surface capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

		// Surface formats
		uint32_t formatCount = 0;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
		}

		// Present modes
		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	VkSurfaceFormatKHR Context::chooseSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const VkSurfaceFormatKHR& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR Context::chooseSwapchainPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const VkPresentModeKHR& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return availablePresentMode;
			}
		}

		// This is the only one guaranteed to be available
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D Context::chooseSwapchainExtent(const VkSurfaceCapabilitiesKHR& capabilities, GLFWwindow* window) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
			return capabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(window, &width, &height);

			VkExtent2D extent = {
				static_cast<uint32_t>(width),
				static_cast<uint32_t>(height)
			};

			extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
			extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

			return extent;
		}
	}

	void Context::createSwapchain() {
		SwapchainSupportDetails support = querySwapchainSupport(physicalDevice_);
		VkSurfaceFormatKHR swapchainSurfaceFormat = chooseSwapchainSurfaceFormat(support.formats);
		VkPresentModeKHR swapchainPresentMode = chooseSwapchainPresentMode(support.presentModes);
		VkExtent2D swapchainExtent = chooseSwapchainExtent(support.capabilities, window_->getHandle());

		// Create swapchain
		VkSwapchainCreateInfoKHR createInfo{
			.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = surface_,
			.minImageCount = support.capabilities.minImageCount,
			.imageFormat = swapchainSurfaceFormat.format,
			.imageColorSpace = swapchainSurfaceFormat.colorSpace,
			.imageExtent = swapchainExtent,
			.imageArrayLayers = 1,
			.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
			.compositeAlpha	= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			.presentMode = swapchainPresentMode
		};
		
		if (vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapchain_) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create swapchain!");
		}
	}

	void Context::getSwapchainImages() {
		uint32_t imageCount = 0;
		vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, nullptr);
		swapchainImages_.resize(imageCount);
		vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, swapchainImages_.data());
	}

	void Context::createSwapchainImageViews() {
		swapchainImageViews_.resize(swapchainImages_.size());

		VkSurfaceFormatKHR surfaceFormat = chooseSwapchainSurfaceFormat(querySwapchainSupport(physicalDevice_).formats);

		for (size_t i = 0; i < swapchainImages_.size(); i++) {
			VkImageViewCreateInfo createInfo{
				.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = swapchainImages_[i],
				.viewType = VK_IMAGE_VIEW_TYPE_2D,
				.format = surfaceFormat.format,
				.components = {
					.r = VK_COMPONENT_SWIZZLE_IDENTITY,
					.g = VK_COMPONENT_SWIZZLE_IDENTITY,
					.b = VK_COMPONENT_SWIZZLE_IDENTITY,
					.a = VK_COMPONENT_SWIZZLE_IDENTITY
				},
				.subresourceRange = {
					.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
					// No mipmapping or multiple layers yet
					.baseMipLevel = 0,
					.levelCount = 1,
					.baseArrayLayer = 0,
					.layerCount = 1
				}
			};

			if (vkCreateImageView(device_, &createInfo, nullptr, &swapchainImageViews_[i]) != VK_SUCCESS) {
				throw std::runtime_error("Failed to create image view!");
			}
		}
	}

	void Context::setupSwapchainResources() {
		createSwapchain();
		getSwapchainImages();
		createSwapchainImageViews();
	}

	void Context::cleanupSwapchainResources() {
		for (VkImageView& imageView : swapchainImageViews_) {
			vkDestroyImageView(device_, imageView, nullptr);
			imageView = VK_NULL_HANDLE;
		}

		for (VkImage& image : swapchainImages_) {
			image = VK_NULL_HANDLE;
		}

		vkDestroySwapchainKHR(device_, swapchain_, nullptr);
		swapchain_ = VK_NULL_HANDLE;
	}

	/* DEPTH RESOURCES */
	VkFormat Context::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
		for (VkFormat format : candidates) {
			VkFormatProperties2 properties;
			vkGetPhysicalDeviceFormatProperties2(physicalDevice_, format, &properties);

			// Require either linear or optimal tiling, as well as all features to be supported
			if (tiling == VK_IMAGE_TILING_LINEAR && (properties.formatProperties.linearTilingFeatures & features) == features) {
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.formatProperties.optimalTilingFeatures & features) == features) {
				return format;
			}

			throw std::runtime_error("Failed to find supported format!");
		}
	}

	void Context::createDepthResources() {
		// Choose format -- needs depth buffer for ordering
		depthFormat_ = findSupportedFormat(config_->depthFormatList, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

		VkExtent2D extent2D = chooseSwapchainExtent(
			querySwapchainSupport(physicalDevice_).capabilities,
			window_->getHandle()
		);

		VkImageCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
			.imageType = VK_IMAGE_TYPE_2D,
			.format = depthFormat_,
			.extent = {.width = extent2D.width, .height = extent2D.height, .depth = 1},
			.mipLevels = 1,
			.arrayLayers = 1,
			.samples = VK_SAMPLE_COUNT_1_BIT,
			.tiling = VK_IMAGE_TILING_OPTIMAL,
			.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
			.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
		};

		
	}

	/* GENERAL */
	void Context::cleanup() {
		cleanupSwapchainResources();
		destroyDevice();
		destroySurface();
		destroyDebugMessenger();
		destroyInstance();
	}
}