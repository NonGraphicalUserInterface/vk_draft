#include "Context.hpp"

#include <stdexcept>
#include <iostream>
#include <cstdint>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Queue.hpp"

namespace VkCore {
	Context::Context(const Config::Config* config) : config_(config) {
		createInstance();
		createDebugMessenger();
		pickPhysicalDevice();
		createDevice();
	}

	Context::~Context() {
		cleanup();
	}

	// Moving
	Context::Context(Context&& other) noexcept :
		config_			(std::exchange(other.config_,			nullptr)),
		instance_		(std::exchange(other.instance_,			VK_NULL_HANDLE)),
		debugMessenger_	(std::exchange(other.debugMessenger_,	VK_NULL_HANDLE)),
		physicalDevice_	(std::exchange(other.physicalDevice_,	VK_NULL_HANDLE)),
		device_			(std::exchange(other.device_,			VK_NULL_HANDLE))
	{}

	Context& Context::operator=(Context&& other) noexcept {
		if (this == &other) return *this;

		cleanup();

		config_ =			std::exchange(other.config_,			nullptr);
		instance_ =			std::exchange(other.instance_,			VK_NULL_HANDLE);
		debugMessenger_ =	std::exchange(other.debugMessenger_,	VK_NULL_HANDLE);
		physicalDevice_ =	std::exchange(other.physicalDevice_,	VK_NULL_HANDLE);
		device_ =			std::exchange(other.device_,			VK_NULL_HANDLE);

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
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

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

	bool Context::isDeviceSuitable(VkPhysicalDevice device) {
		//// Properties: what IS device? Features: what HAS device?
		//VkPhysicalDeviceProperties deviceProperties;
		//VkPhysicalDeviceFeatures deviceFeatures;
		//vkGetPhysicalDeviceProperties(device, &deviceProperties);
		//vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

		//return true; // For now, Vulkan support is enough.
		
		return findQueueFamilies(device).isComplete();
	}

	/* LOGICAL DEVICE */
	void Context::createDevice() {
		// Logical device needs to know what queue families it can use
		QueueFamilyIndices indices = findQueueFamilies(physicalDevice_);

		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;

		float queuePriority = 1.0f;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		// We will come back to device features later
		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pQueueCreateInfos = &queueCreateInfo;
		createInfo.queueCreateInfoCount = 1;
		createInfo.pEnabledFeatures = &deviceFeatures;

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
	}

	void Context::destroyDevice() {
		if (device_ != VK_NULL_HANDLE) {
			vkDestroyDevice(device_, nullptr);
			device_ = VK_NULL_HANDLE;
		}
	}

	/* GENERAL */
	void Context::cleanup() {
		destroyDevice();
		destroyDebugMessenger();
		destroyInstance();
	}
}