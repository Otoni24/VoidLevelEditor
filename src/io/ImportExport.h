#pragma once

#include <iostream>
#include <optional>
#include "level/Level.h"
#include "project/Project.h"

namespace vle {
    namespace ImportExport
    {
        bool save(const Project& project, const std::string& path);
        std::optional<Project> load(const std::string& path);
        bool exportLevel(const Level& level, const std::string& path);
    }
}