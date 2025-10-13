#pragma once

#include "Vectorizer/Vectorizer.h"
#include "GameObject.h"
#include "core/Utils.h"

namespace vle {
	struct Level
	{
		std::string levelNameId;
		List<sf::VertexArray> hitboxMap;
		List<unique<GameObject>> gameObjects;
		void addGameObject(unique<GameObject> object)
		{
			gameObjects.push_back(std::move(object));
		}
	};
}