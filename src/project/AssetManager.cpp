#include "AssetManager.h"

std::unique_ptr<AssetManager> AssetManager::assetManager = nullptr;
AssetManager& AssetManager::Get()
{
	if (!assetManager)
	{
		assetManager = unique<AssetManager>{ new AssetManager };

	}
	return *assetManager;
}

bool AssetManager::LoadTexture(const std::string& name,const std::string& path)
{
	if (mLoadedTextures.count(name))
	{
		return false;
	}
	auto newTexture = std::make_unique<sf::Texture>();
	if (newTexture->loadFromFile(path))
	{
		mLoadedTextures[name] = std::move(newTexture);
		return true;
	}
	return false;
}

const sf::Texture* AssetManager::GetTexture(const std::string& name) const
{
	auto found = mLoadedTextures.find(name);
	if (found != mLoadedTextures.end())
	{
		return found->second.get();
	}
	return nullptr;
}

bool AssetManager::RemoveTexture(const std::string& name)
{
	return mLoadedTextures.erase(name) > 0;
}