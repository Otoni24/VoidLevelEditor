#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include "core/Utils.h"
#include "project/Asset.h"

class AssetManager
{
public:
	static AssetManager& Get();

	AssetManager(const AssetManager&) = delete;
	AssetManager& operator=(const AssetManager&) = delete;
	AssetManager(AssetManager&&) = delete;
	AssetManager& operator=(AssetManager&&) = delete;

	bool LoadTexture(std::string& name, std::string& path);
	const sf::Texture* GetTexture(const std::string& name) const;
	bool RemoveTexture(const std::string& name);
	//Dictionary<std::string, unique<sf::Texture>>& GetLoadedTextures() const;
private:
	AssetManager() = default;
	static unique<AssetManager> assetManager;
	Dictionary<std::string, unique<sf::Texture>> mLoadedTextures;
};