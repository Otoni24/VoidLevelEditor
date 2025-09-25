cmake_minimum_required(VERSION 3.28)
project(VoidLevelEditor LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(
    VectorizerLib
    GIT_REPOSITORY https://github.com/Otoni24/VectorizerLib.git
    GIT_TAG "master"
    GIT_SHALLOW ON
)
set(VEC_IMPLEMENT_STB_IMAGE OFF CACHE BOOL "Let VectorizerLib provide the stb_image implementation" FORCE)
FetchContent_MakeAvailable(VectorizerLib)

FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    )
FetchContent_MakeAvailable(SFML)

FetchContent_Declare(ImGui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG v1.91.1-docking
    GIT_SHALLOW ON
    )
FetchContent_MakeAvailable(ImGui)
FetchContent_GetProperties(ImGui SOURCE_DIR IMGUI_DIR)

set(IMGUI_SFML_FIND_SFML OFF)
FetchContent_Declare(ImGui-SFML
    GIT_REPOSITORY https://github.com/SFML/imgui-sfml
    GIT_TAG v3.0
    GIT_SHALLOW ON
    )
FetchContent_MakeAvailable(ImGui-SFML)

FetchContent_Declare(nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
    GIT_SHALLOW ON
)
FetchContent_MakeAvailable(nlohmann_json)

add_executable(VoidLevelEditor WIN32
    "src/main.cpp"
    "src/core/Application.cpp"
    "src/io/ImportExport.cpp"
    "src/project/AssetManager.cpp"
    "lib/ImGuiFileDialog/ImGuiFileDialog.cpp"
 )

target_include_directories(VoidLevelEditor PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/lib/ImGuiFileDialog)
target_compile_features(VoidLevelEditor PRIVATE cxx_std_17)
target_link_libraries(VoidLevelEditor PRIVATE VectorizerLib SFML::Graphics SFML::Main ImGui-SFML::ImGui-SFML nlohmann_json::nlohmann_json)