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

bool AssetManager::LoadAsset(std::string& name, std::string& path)
{
	if (mLoadedAssets.count(name))
	{
		return false;
	}
	auto newAsset = std::make_unique<Asset>();
	if (newAsset->texture.loadFromFile(path))
	{
		newAsset->name = name;
		mLoadedAssets[name] = std::move(newAsset);
		return true;
	}
	return false;
}

const Asset* AssetManager::GetAsset(const std::string& name) const
{
	auto found = mLoadedAssets.find(name);
	if (found != mLoadedAssets.end())
	{
		return found->second.get();
	}
	return nullptr;
}

bool AssetManager::RemoveAsset(const std::string& name)
{
	return mLoadedAssets.erase(name) > 0;
}

const Dictionary<std::string, unique<Asset>>& AssetManager::GetLoadedAssets() const
{
	return mLoadedAssets;
}
