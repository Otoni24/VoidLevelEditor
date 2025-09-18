#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include "core/Utils.h"
#include "project/Asset.h"
#include "level/Level.h"

struct Project
{
	Level level;
	std::string backgroundTexturePath;
	std::string hitboxTexturePath;
	int simplifyIndex;
	bool bHitboxMap;
	List<Asset> assets;
	void addAsset(Asset asset)
	{
		assets.push_back(asset);
	}
	Project()
		: simplifyIndex{3},
		bHitboxMap{false}
	{}
};