#include "App.hpp"

#include <iostream>

namespace App {
	App::App(const Config::Config& config) :
		window_(config),
		context_(&config)
	{
		std::cout << "Initialised App" << std::endl;
	}

	void App::run() {
		initVulkan();
		mainLoop();
		cleanup();
	}

	void App::initVulkan() {
		// Do nothing for now
	}

	void App::mainLoop() {
		while (!window_.shouldClose()) {
			window_.pollEvents();
		}
	}

	void App::cleanup() {

	}
}