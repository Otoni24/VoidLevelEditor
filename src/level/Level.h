#pragma once
#include <vector>
#include "GameObject.h"

struct Level
{
	std::vector<GameObject> mGameObjects;
	void addGameObject(GameObject object)
	{
		mGameObjects.push_back(object);
	}
};