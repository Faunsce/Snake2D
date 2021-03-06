#include "LoseMode.hpp"
#include <iostream>

LoseMode::LoseMode(std::mutex* mutex, sf::Image ss) : screenShot{ ss }, Mode("LoseMode.json", mutex, ModeOption::Paused) {
	gameScreen.loadFromImage(screenShot);
	screenObjectsMap["game"].setTexture(&gameScreen);
}

std::pair<ModeAction, ModeOption> LoseMode::Run(sf::Time time, sf::RenderWindow& window) {
	// Handle Input
	sf::Event evnt;
	while (window.pollEvent(evnt)) {
		switch (evnt.type)
		{
		case sf::Event::KeyPressed:
			switch (evnt.key.code)
			{
			case sf::Keyboard::Escape:
				return std::make_pair(ModeAction::DropTo, ModeOption::None);
			case sf::Keyboard::Enter:
				return std::make_pair(ModeAction::DropTo, ModeOption::Menu);
			default:
				break;
			}
		default:
			break;
		}
	}
	// Handle Passive Logic
	if (timeBank.asSeconds() > .01) {

	}
	else {
		timeBank += time;
	}
	// Im case of no state changes
	return std::make_pair(ModeAction::None, ModeOption::None);
}