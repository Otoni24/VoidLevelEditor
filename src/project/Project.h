#pragma once

#include <vector>
#include <SFML/Graphics.hpp>
#include "Vectorizer/Vectorizer.h"
#include "core/Utils.h"
#include "project/Asset.h"
#include "level/Level.h"

namespace vle {
	struct Project
	{
		Level level;
		std::string backgroundTexturePath;
		std::string hitboxTexturePath;
		int simplifyIndex;
		bool bHitboxMap;
		Map<std::string, unique<Asset>> assets;
		Project()
			: simplifyIndex{ 3 },
			bHitboxMap{ false }
		{
		}
		Project(const Project&) = delete;
		Project& operator=(const Project&) = delete;
		Project(Project&&) = default;
		Project& operator=(Project&&) = default;
	};
}