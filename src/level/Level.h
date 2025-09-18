#pragma once

#include "Vectorizer/Vectorizer.h"
#include "GameObject.h"
#include "core/Utils.h"

struct Level
{
	Vectorizer::Math::Chain hitBoxChain;
	List<GameObject> gameObjects;
	void addGameObject(GameObject object)
	{
		gameObjects.push_back(object);
	}
};