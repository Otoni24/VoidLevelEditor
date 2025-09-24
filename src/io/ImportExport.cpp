#include "ImportExport.h"
#include "io/Serialization.h"
#include <fstream>
#include <iostream>

using namespace vle;

using json = nlohmann::json;

bool ImportExport::save(const Project& project, const std::string& path)
{
	std::ofstream fileStream(path);
    if (!fileStream.is_open())
    {
        std::cerr << "Error: output file invalid" << path << std::endl;
        return false;
    }
    json projectJson = project;
    fileStream << projectJson.dump(4);
    fileStream.close();
    return true;
}

std::optional<Project> ImportExport::load(const std::string& path)
{
    std::ifstream fileStream(path);
    if (!fileStream.is_open())
    {
        std::cerr << "Error: input file invalid:" << path << std::endl;
        return std::nullopt;
    }
    json projectJson;
    try
    {
        fileStream >> projectJson;
    }
    catch (json::parse_error& e)
    {
        std::cerr << "JSON Parsing Error:" << e.what() << std::endl;
        return std::nullopt;
    }
    try
    {
        return projectJson.get<Project>();
    }
    catch (json::exception& e)
    {
        std::cerr << "JSON Conversion Error:" << e.what() << std::endl;
        return std::nullopt;
    }
}

bool ImportExport::exportLevel(const Level& level, const std::string& path)
{
    std::ofstream fileStream(path);
    if (!fileStream.is_open())
    {
        std::cerr << "Error: output file invalid" << path << std::endl;
        return false;
    }
    json levelJson = level;
    fileStream << levelJson.dump(4);
    fileStream.close();
    return true;
}
