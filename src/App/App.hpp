#include "Window/Window.hpp"

class App {
public:
	App() = default;
	~App() = default;

	App(const App&) = delete;
	App& operator=(const App&) = delete;
	App(App&&) = delete;
	App& operator=(App&&) = delete;

	// Methods
	void run();
private:
	void initVulkan();
	void mainLoop();
	void cleanup();

	Window window_;
};