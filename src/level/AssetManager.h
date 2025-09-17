#pragma once
#include <iostream>
#include <SFML/Graphics.hpp>
#include "core/Utils.h"
#include "level/Asset.h"

class AssetManager
{
public:
	static AssetManager& Get();
	bool LoadAsset(std::string& name, std::string& path);
	const Asset* GetAsset(const std::string& name) const;
	bool RemoveAsset(const std::string& name);
	const Dictionary<std::string, unique<Asset>>& GetLoadedAssets() const;
private:
	AssetManager() = default;
	static unique<AssetManager> assetManager;
	Dictionary<std::string, unique<Asset>> mLoadedAssets;
};