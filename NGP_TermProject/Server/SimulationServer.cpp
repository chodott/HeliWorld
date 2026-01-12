#include "SimulationServer.h"
#include "ProtocolConstants.h"

SimulationServer::SimulationServer(ClientInputBuffer& inputBuffer, SnapshotPacketBuffer& packetBuffer) 
	: clientInputBuffer(inputBuffer), snapshotPacketBuffer(packetBuffer)
{
	for (int i = 0; i < Protocol::kMaxPlayerCount; ++i)
	{
		m_player[i] = new CPlayer();
		m_player[i]->SetPosition(initialPos[i]);
		m_player[i]->RotatePYR(initialRot[i]);
	}
	for (int i = 0; i < Protocol::kMaxItemCount; i++)
	{
		m_ItemObject[i] = new CItemObject();
		m_ItemObject[i]->SetPosition(0,0,0);
	}

	UpdateSnapshot(0);
}

SimulationServer::~SimulationServer()
{
	for (int i = 0; i < Protocol::kMaxItemCount; i++)
	{
		if (m_ItemObject[i] != nullptr)
		{
			delete m_ItemObject[i];
		}
	}
}

void SimulationServer::Update()
{
	uint64_t resimulateStartTick = clientInputBuffer.GetResimulateStartTick(GetTick());
	uint64_t resetTick = resimulateStartTick;
	if (resimulateStartTick > 0)
	{
		resetTick--;
	}
	ResetToSnapshot(resetTick);

	PlayerKeyPacket keyPacket;
	for (uint64_t tick = resimulateStartTick; tick <= GetTick(); ++tick)
	{
		for (int i = 0; i < Protocol::kMaxPlayerCount; ++i)
		{
			CPlayer* player = m_player[i];
			clientInputBuffer.TryGetKeyPacket(i, tick, keyPacket);
			player->Update(Protocol::kFixedTick);
		}

		CheckCollision();

		while (!trashCan.empty())
		{
			trashCan.front()->Deactivate();
			trashCan.pop();
		}
		UpdateSnapshot(tick);
		GenerateMissileEvents(tick);
	}



	//for (int i = 0; i < MAX_CLIENT_NUM; ++i)
	//{
	//	CPlayer* player = clients[i]->m_player;
	//	if (clients[i]->keyPacket_q.try_pop(player->keyPacket));
	//	// connected, but dead
	//	if (clients[i]->IsConnected() && !player->IsActive())
	//	{
	//		clients[i]->deadTime += elapsedTime;
	//		if (clients[i]->deadTime > RESPAWN_TIME)
	//		{
	//			player->SetActive(true);
	//			clients[i]->deadTime = 0.f;
	//		}
	//	}
	//}

	itemSpawnTime += elapsedTime;
	if (itemSpawnTime > itemRespawnTime)
	{
		itemSpawnTime -= itemRespawnTime;
		SpawnItem();
	}

	++serverTick;
	PreparePackets();
}

void SimulationServer::CheckCollision()
{
	for (int i = 0; i < Protocol::kMaxPlayerCount; ++i)
	{
		CPlayer* iPlayer = m_player[i];
		if (!iPlayer->IsActive())
			continue;

		//Collision Map
		if (iPlayer->m_fyPos < MIN_BOUNDARY_Y || iPlayer->m_fyPos > MAX_BOUNDARY_Y ||
			iPlayer->m_fzPos < MIN_BOUNDARY_Z || iPlayer->m_fzPos > MAX_BOUNDARY_Z ||
			iPlayer->m_fxPos < MIN_BOUNDARY_X || iPlayer->m_fxPos > MAX_BOUNDARY_X)
		{
			iPlayer->SetPosition(iPlayer->m_fOldxPos, iPlayer->m_fOldyPos, iPlayer->m_fOldzPos);
		}

		for (int j = 0; j < Protocol::kMaxPlayerCount; ++j)
		{
			//Same Player
			if (i == j)
				continue;

			CPlayer* jPlayer = m_player[j];
			if (!jPlayer->IsActive())
				continue;

			// Check Player to Player
			if (iPlayer->GetBoundingBox().Intersects(jPlayer->GetBoundingBox()))
			{
				//iPlayer->m_nHp -= 10;
				iPlayer->SetPosition(iPlayer->m_fOldxPos, iPlayer->m_fOldyPos, iPlayer->m_fOldzPos);

				//jPlayer->m_nHp -= 10;
				jPlayer->SetPosition(jPlayer->m_fOldxPos, jPlayer->m_fOldyPos, jPlayer->m_fOldzPos);
			}

			for (auto& missile : jPlayer->m_pMissiles)
			{
				if (!missile->IsActive())
					continue;

				// Check Players and Missiles
				if (iPlayer->GetBoundingBox().Intersects(missile->GetBoundingBox()))
				{
					iPlayer->m_nHp -= missile->damage;
					missile->ShouldDeactive();
					trashCan.push(missile);
					if (iPlayer->m_nHp <= 0)
					{
						iPlayer->Reset(i);
						iPlayer->ShouldDeactive();
					}
				}
			}
		}

		for (int j = 0; j < Protocol::kMaxItemCount; j++)
		{
			if (!m_ItemObject[j]->IsActive())		continue;

			if (iPlayer->GetBoundingBox().Intersects(m_ItemObject[j]->GetBoundingBox()))
			{
				iPlayer->m_nHp += m_ItemObject[j]->healAmount;
				if (iPlayer->m_nHp > iPlayer->maxHp)			iPlayer->m_nHp = iPlayer->maxHp;

				m_ItemObject[j]->ShouldDeactive();
				trashCan.push(m_ItemObject[j]);
			}
		}
	}
}


void SimulationServer::SpawnItem()
{
	for (int i = 0; i < Protocol::kMaxItemCount; ++i)
	{
		if (!m_ItemObject[i]->IsActive())
		{
			m_ItemObject[i]->SetActive(true);
			m_ItemObject[i]->healAmount = ((rand() % 3) + 1) * 10;
			m_ItemObject[i]->SetPosition(rand() % MAX_BOUNDARY_X,
				rand() % (MAX_BOUNDARY_Y - MIN_BOUNDARY_Y) + MIN_BOUNDARY_Y,
				rand() % MAX_BOUNDARY_Z);
			break;
		}
	}
}


void SimulationServer::ResetToSnapshot(uint64_t targetTick)
{
	auto it = SnapshotLogMap.find(targetTick);
	if (it == SnapshotLogMap.end())
	{
		return;
	}

	ServerSnapshot& snapshot = it->second;
	for (int i = 0; i < Protocol::kMaxPlayerCount; ++i)
	{
		CPlayer* player = m_player[i];
		player->SetPosition(snapshot.playerSnapshots[i].position);
		player->RotatePYR(snapshot.playerSnapshots[i].rotation);
		player->SetHp(snapshot.playerSnapshots[i].hp);

		int startMissileIndex = i * Protocol::kMaxMissileCountPerPlayer;
		for (int j = 0; j < Protocol::kMaxMissileCountPerPlayer; ++j)
		{
			CMissileObject* missile = player->m_pMissiles[j];
			missile->SetPosition(snapshot.missileSnapshots[startMissileIndex + j].position);
			missile->SetLifeTime(snapshot.missileSnapshots[startMissileIndex + j].lifeTime);
			missile->SetActive(snapshot.missileSnapshots[startMissileIndex + j].active);
		}
	}

	for (int i = 0; i < Protocol::kMaxItemCount; ++i)
	{
		m_ItemObject[i]->SetPosition(snapshot.itemSnapshots[i].position);
	}

}

void SimulationServer::UpdateSnapshot(uint64_t targetTick)
{
	auto it = SnapshotLogMap.find(targetTick);
	if (it == SnapshotLogMap.end())
	{
		it = SnapshotLogMap.emplace(targetTick, ServerSnapshot{}).first;
	}
	ServerSnapshot& snapshot = it->second;
	for (int i = 0; i < Protocol::kMaxPlayerCount; ++i)
	{
		CPlayer* player = m_player[i];
		snapshot.playerSnapshots[i].position = player->GetCurPos();
		snapshot.playerSnapshots[i].rotation = player->GetCurRot();
		snapshot.playerSnapshots[i].hp = player->GetHp();

		int startMissileIndex = i * Protocol::kMaxMissileCountPerPlayer;
		for (int j = 0; j < Protocol::kMaxMissileCountPerPlayer; ++j)
		{
			CMissileObject* missile = player->m_pMissiles[j];
			snapshot.missileSnapshots[startMissileIndex + j].position = missile->GetCurPos();
			snapshot.missileSnapshots[startMissileIndex + j].lifeTime = missile->GetLifeTime();
			snapshot.missileSnapshots[startMissileIndex + j].active = missile->IsActive();
		}
	}

	for (int i = 0; i < Protocol::kMaxItemCount; ++i)
	{
		snapshot.itemSnapshots[i].position = m_ItemObject[i]->GetCurPos();
	}
}

void SimulationServer::GenerateMissileEvents(uint64_t tick)
{
	if (tick == 0) return;

	for (int playerNum = 0; playerNum < Protocol::kMaxPlayerCount; ++playerNum)
	{
		CPlayer* player = m_player[playerNum];
		ServerSnapshot& snapshot = SnapshotLogMap.at(tick - 1);
		for (int i = 0; i < Protocol::kMaxMissileCountPerPlayer; ++i)
		{
			CMissileObject* missile = player->m_pMissiles[i];
			bool prevMissileActive = snapshot.missileSnapshots[playerNum * Protocol::kMaxMissileCountPerPlayer + i].active;
			bool curMissileActive = missile->IsActive();
			if (prevMissileActive == curMissileActive)
			{
				continue;
			}

			if (curMissileActive == true && client->ShouldSendEvent(missile->GetID()) == false)
			{
				continue;
			}

			GetQueue<LocalMissileEventPacket>().push(
				{
					CS_LocalMissileEvent,
					missile->GetID(),
					tick,
					playerNum,
					missile->IsActive()
				}
			);
		}
	}
}

void SimulationServer::PreparePackets()
{
	TickSnapshotPacket tickSnapshot{};

	for (int clientNum = 0; clientNum < Protocol::kMaxPlayerCount; ++clientNum)
	{
		Client* client = clients[clientNum];
		CPlayer* player = client->m_player;
		tickSnapshot.playerInfos[clientNum] = { clientNum, player->m_nHp, player->GetCurPos(), player->GetCurRot(), player->IsActive() };

		for (int i = 0; i < Protocol::kMaxMissileCountPerPlayer; ++i)
		{
			CMissileObject* missile = player->m_pMissiles[i];
			MissileInfoPacket& missileInfo = tickSnapshot.missileInfos[clientNum * Protocol::kMaxMissileCountPerPlayer + i];
			missileInfo.position = missile->GetCurPos();
			missileInfo.active = missile->IsActive();
		}
	}

	for (int i = 0; i < Protocol::kMaxItemCount; ++i)
	{
		CItemObject* item = m_ItemObject[i];
		ItemInfoPacket& itemInfo = tickSnapshot.itemInfos[i];
		ConvertFloat3toInt32(item->GetCurPos(), itemInfo.positionX, itemInfo.positionY, itemInfo.positionZ, MAP_SCALE);
		itemInfo.active = item->IsActive();
	}

	snapshotPacketBuffer.PushSnapshotPacket(tickSnapshot);
}