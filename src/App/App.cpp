#include "App.hpp"

void App::run() {
	window_.init();
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
	window_.cleanup();
}