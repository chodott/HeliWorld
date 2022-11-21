#include "GameObjectMgr.h"

void GameObjectMgr::AnimateObjects()
{
}

void GameObjectMgr::CheckCollision()
{
	if (pMissleObject && PlayerObject)
	{
		for (int i = 0; i < sizeof(PlayerObject); ++i)
		{
			for (int j = 0; j < sizeof(pMissleObject); ++j)
			{
				if (PlayerObject[i]->GetActive() && pMissleObject[j]->GetActive()) {
					if (PlayerObject[i]->Player_id != pMissleObject[j]->Player_id) {
						if (PlayerObject[i]->m_xmOOBB.Intersects(pMissleObject[j]->m_xmOOBB))
						{
							PlayerObject[i]->SetActive(FALSE);
							pMissleObject[j]->SetActive(FALSE);
							break;
						}
					}
				}
			}
		}
	}
}
