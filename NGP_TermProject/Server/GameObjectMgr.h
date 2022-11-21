#pragma once
#include "Server.h"
#include "GameObject.h"

class GameObjectMgr
{

public:
	
	GameObject** pMissleObject;
	CPlayer** PlayerObject;
	void AnimateObjects();
	void CheckCollision();
};