#pragma once

#include <iostream>
#include <optional>
#include "Level.h"

namespace LevelUtils
{
    bool save(const Level& level, const std::string& path);
    std::optional<Level> load(const std::string& path);
}