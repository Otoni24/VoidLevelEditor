#pragma once

#include <iostream>
#include <SFML/Graphics.hpp>

struct GameObject
{
	std::string assetID;
	sf::Sprite sprite;
	GameObject(std::string inputAssetID, sf::Sprite inputSprite)
		: assetID{ inputAssetID }, sprite{ inputSprite }
	{ }
};