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
	Map<std::string, Asset> assets;
	Project()
		: simplifyIndex{ 3 },
		bHitboxMap{ false }
	{
	}
};