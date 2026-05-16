#include <iostream>
#include <App/App.hpp>
#include <Config/Config.hpp>

int main() {
	Config::Config config = {
		.width = 800,
		.height = 600,
		.title = "Vulkan Draft App"
	};

	App::App app(config);

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}