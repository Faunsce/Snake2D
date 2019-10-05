// Preprocessor
#include <SFML/Graphics.hpp> 
#include <iostream> 
#include <string>
#include <vector>
#include <stack>
#include <thread>
#include <mutex>
#include <atomic>
#include <Player.hpp>
#include <ModeList.hpp>
#include <RuntimeStats.hpp>
#include <Settings.hpp>
// Entry Point
int main() {
	static std::mutex mu;
	// Setup Window
	auto desktop = sf::VideoMode::getDesktopMode();
	desktop.width += 1;
	sf::RenderWindow window(desktop, "Cnake", sf::Style::None);
	window.setActive(false);
	// Load Settings
	Settings gameSettings(&window);
	// Prepare Stack
	std::stack<std::unique_ptr<Mode>> ModeStack;
	ModeStack.push(std::make_unique<IntroMode>(&mu));
	// Variable for killing thread safely
	std::atomic<bool> isRunning = true;
	std::atomic<bool> showStats = true;
	std::atomic<bool> isPaused = false;
	std::atomic<bool> isWaiting = false;
	// Create Rendering Thread
	// TODO: Fix weird rendering error that occurs when re-pushing old states
	std::thread RenderThread([&window, &ModeStack, &isRunning, &showStats, &isPaused, &isWaiting] {
		// Window Settings
		window.setActive(true);
		//window.setFramerateLimit(60);
		// Setup Stats
		RuntimeStats stats;
		// Render Loop
		while (isRunning) {
			if (!isPaused) {
				// Rendering
				window.clear();
				mu.lock();
				for (const auto& element : ModeStack.top()->screenObjects) {
					window.draw(*element);
				}
				if (showStats) {
					stats.draw(window);
				}
				window.display();
				mu.unlock();
				stats.update();
			} else {
				window.setActive(false);
				isWaiting = true;
				while (true) {
					if (!isPaused) {
						window.setActive(true);
						break;
					}
				}
			}
		}
		std::cout << "Rendering thread closed!" << std::endl;
	});
	// Begin Game
	sf::Clock GameClock;
	while (!ModeStack.empty()) {
		sf::Time timePassed = GameClock.restart();
		auto result = ModeStack.top()->Run(timePassed, window);
		if (result.first != ModeAction::None) {
			sf::Event evnt;
			while (window.pollEvent(evnt));
			switch (result.first)
			{
			case ModeAction::Add:
				switch (result.second)
				{
				case ModeOption::Intro:
					ModeStack.push(std::make_unique<IntroMode>(&mu));
					break;
				case ModeOption::Menu:
					ModeStack.push(std::make_unique<MenuMode>(&mu));
					break;
				case ModeOption::Settings:
					ModeStack.push(std::make_unique<SettingsMode>(&mu, &gameSettings));
					break;
				case ModeOption::Game:
					ModeStack.push(std::make_unique<GameMode>(&mu, gameSettings.getGameSpeed()));
					break;
				case ModeOption::Paused:
				case ModeOption::Lose:
					isPaused = true;
					while (!isWaiting) {
					}
					window.setActive(true);
					showStats = false;
					if (result.second == ModeOption::Paused) {
						ModeStack.push(std::make_unique<PausedMode>(&mu, window.capture()));
					} else {
						ModeStack.push(std::make_unique<LoseMode>(&mu, window.capture()));
					}
					window.setActive(false);
					isPaused = false;
					break;
				}
				break;
			case ModeAction::DropTo:
				// Safely Kill Render Thread 
				if ((result.second == ModeOption::None) || (result.second == ModeOption::One && ModeStack.size() == 1)) {
					isRunning = false;
					RenderThread.join();
				} 
				mu.lock();
				if (ModeStack.top()->type() == ModeOption::Paused) {
					showStats = true;
				}
				if (result.second == ModeOption::One) {
					ModeStack.pop();
				} else {
					while (!(ModeStack.empty()) && (ModeStack.top()->type() != result.second)) {
						ModeStack.pop();
					}
				}
				if (ModeStack.empty()) {
					window.close();
				}
				mu.unlock();
				isRunning = true;
				break;
			default:
				break;
			}
		}
	}
	gameSettings.saveToFile();
	std::cout << "Application closing" << std::endl;
	//std::cin.ignore(1);
}

