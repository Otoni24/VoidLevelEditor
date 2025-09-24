#pragma once
#include <string>
#include <SFML/Graphics.hpp>

namespace vle {
	struct AssetData
	{
		std::string name;
		std::string texturePath;
		sf::Vector2f defaultScale;
		sf::Angle defaultRotation;
	};
	struct Asset
	{
		std::string texturePath;
		sf::Vector2f defaultScale = { 1.0f, 1.0f };
		sf::Angle defaultRotation;
		Asset() = default;
		~Asset() = default;
		Asset(const Asset&) = default;
		explicit Asset(const AssetData& other)
			: texturePath(other.texturePath),
			defaultScale(other.defaultScale),
			defaultRotation(other.defaultRotation)
		{
		}
		Asset& operator=(const Asset&) = default;
		Asset& operator=(const AssetData& other)
		{
			texturePath = other.texturePath;
			defaultScale = other.defaultScale;
			defaultRotation = other.defaultRotation;
			return *this;
		}
	};
}