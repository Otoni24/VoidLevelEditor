#pragma once

#include "Vectorizer/Vectorizer.h"
#include "GameObject.h"
#include "core/Utils.h"

struct Level
{
	Vectorizer::Math::Chain hitBoxChain;
	List<unique<GameObject>> gameObjects;
	void addGameObject(std::unique_ptr<GameObject> object)
	{
		gameObjects.push_back(std::move(object));
	}
};