#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class Screen : public sf::Drawable
{
public:
	std::vector<sf::Drawable> screenElements;
};