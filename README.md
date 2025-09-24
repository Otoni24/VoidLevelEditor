# VoidLevelEditor

A simple and visual 2D level editor, written in C++, for creating levels and hitbox data for your game.

It allows for the visual placement of assets onto a background image, and features simple, automatic collision generation from a png b&w image.

-----

## Features

* **Visual Layout:** Place, move, scale, and rotate game assets directly on your level's background image for an intuitive workflow.
* **Automatic Hitbox Generation:** Powered by the integrated `VectorizerLib`, the editor can process a black and white image of your level to automatically trace and generate polygonal hitbox chains, which are then visible and editable.
* **Project and Level Management:** Save your entire workspace, including asset definitions and paths, into a project file for later editing. When ready, export the finalized level data into a clean JSON format for your game.
* **Modern C++ Stack:** Built with standard C++17, using SFML for rendering and a simple, dockable user interface created with ImGui.

-----

## Workflow Overview

The editor is designed to streamline the process of creating a level. The typical workflow involves creating a new project, defining the assets (sprites) and hitbox map, and then populating the scene by dragging objects from the Asset Library into the Level Viewport.

**Important:** To preserve your work for future modifications, always use the **"Save Project"** function. The "Export Level" command generates a simplified `.json` file intended only for game consumption, which **cannot be re-imported** into the editor.

-----

## Game Integration

To load the exported `.json` levels into your C++ game, `VoidLevelEditor` is designed to be used with its companion library, `VoidLevelLoader`.

This is a separate, lightweight, and standalone library that parses the exported JSON and converts it into a simple C++ data structure, ready for you to instantiate your game objects.

The loader is available at its own repository and is designed for easy integration via CMake `FetchContent`:
* **[VoidLevelLoader](https://github.com/Otoni24/VoidLevelLoader.git)**

-----

## Dependencies

* [SFML](https://github.com/SFML/SFML)
* [DearImGui](https://github.com/ocornut/imgui)
* [ImGui-SFML](https://github.com/SFML/imgui-sfml)
* [nlohmann/json](https://github.com/nlohmann/json)
* [VectorizerLib](https://github.com/Otoni24/VectorizerLib) -Personal Library

-----

## License

This project is licensed under the MIT License. See the `LICENSE` file for more details.