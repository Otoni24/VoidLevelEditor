#pragma once
#include <string>
#include <SFML/Graphics.hpp>

struct AssetData
{
	std::string name;
	std::string texturePath;
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
		: texturePath(other.texturePath)
	{ }
	Asset& operator=(const Asset&) = default;
	Asset& operator=(const AssetData& other)
	{
		texturePath = other.texturePath;
		defaultScale = { 1.0f, 1.0f };
		return *this;
	}
};