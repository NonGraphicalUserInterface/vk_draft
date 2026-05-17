#include <Config/Config.hpp>
#include <Window/Window.hpp>

#include <VkCore/Context.hpp>

namespace App {
	class App {
	public:
		App(const Config::Config& config);
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

		Window::Window window_;
		VkCore::Context context_;
	};
}