#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>

namespace vle {
	struct GameObject
	{
		std::string assetID;
		std::optional<sf::Sprite> sprite;
	};
}