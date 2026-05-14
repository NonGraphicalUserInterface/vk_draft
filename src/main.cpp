#include <iostream>
#include "App/App.hpp"

int main() {
	App app;

	try {
		std::cout << "running" << std::endl;
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}