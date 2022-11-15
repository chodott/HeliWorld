#pragma once
#include "Server.h"
#include "GameObject.h"

class GameObjectMgr
{

public:
	std::vector<GameObject*> Objects;

	void AnimateObjects();
	void CheckCollision();
};