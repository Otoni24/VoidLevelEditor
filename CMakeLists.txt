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
FetchContent_MakeAvailable(VectorizerLib)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 3.0.1
    GIT_SHALLOW ON
    )
FetchContent_MakeAvailable(SFML)
FetchContent_Declare(ImGui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG v1.91.1
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

add_executable(VoidLevelEditor
    
 "src/main.cpp")

target_compile_features(VoidLevelEditor PRIVATE cxx_std_17)
target_link_libraries(VoidLevelEditor PRIVATE VectorizerLib SFML::Graphics ImGui-SFML::ImGui-SFML)