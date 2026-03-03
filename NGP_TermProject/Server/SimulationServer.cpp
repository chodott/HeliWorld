#include "SimulationServer.h"
#include "ProtocolConstants.h"

XMFLOAT3 SimulationServer::initialPos[Protocol::kMaxPlayerCount]{ {100,400,100},{900, 400, 900},{900.0f, 400.0f, 100.0f},{100.0f, 400.0f, 900.0f} };
XMFLOAT3 SimulationServer::initialRot[Protocol::kMaxPlayerCount]{ {0,0,0},{0,0,0},{0,0,0},{0,0,0} };

SimulationServer::SimulationServer(array<ClientInputBuffer, Protocol::kMaxPlayerCount>& inputBuffer, SnapshotPacketBuffer& packetBuffer, NetworkEventQueue& networkEventQueue)
	: clientInputBuffer(inputBuffer), snapshotPacketBuffer(packetBuffer), eventQueue(networkEventQueue)
{
	for (int i = 0; i < Protocol::kMaxPlayerCount; ++i)
	{
		m_player[i] = new CPlayer();
		m_player[i]->SetPosition(initialPos[i]);
		m_player[i]->RotatePYR(initialRot[i]);
		m_player[i]->SetActive(true);
	}
	for (int i = 0; i < Protocol::kMaxItemCount; i++)
	{
		m_ItemObject[i] = new CItemObject();
		m_ItemObject[i]->SetPosition(0, 0, 0);
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

void SimulationServer::Update(const float elapsedTime)
{
	uint64_t curTick = GetTick();
	uint64_t resimulateStartTick = GetResimulateStartTick(curTick);
	resimulateStartTick = resimulateStartTick > 0 ? resimulateStartTick - 1 : 0;
	ResetToSnapshot(resimulateStartTick);

	for (uint64_t tick = resimulateStartTick; tick <= curTick; ++tick)
	{
		ProcessPlayerInputs(tick, elapsedTime);

		CheckCollision();

		ProcessDeactivation();

		UpdateSnapshot(tick);
		GenerateMissileEvents(tick);
	}

	SpawnItem(elapsedTime);

	++serverTick;
	SaveLatestSnapshot();
	CleanSnapshotLogs();
}

uint64_t SimulationServer::GetResimulateStartTick(const uint64_t curTick)
{
	uint64_t earliestTick = curTick;
	for(int i = 0; i < Protocol::kMaxPlayerCount; ++i)
	{
		uint64_t tick = clientInputBuffer[i].GetResimulateStartTick(curTick);
		earliestTick = min(earliestTick, tick);
	}
	return earliestTick;
}

void SimulationServer::ProcessPlayerInputs(const uint64_t tick, const float elapsedTime)
{
	for (int i = 0; i < Protocol::kMaxPlayerCount; ++i)
	{
		CPlayer* player = m_player[i];

		PlayerInputPacket input;
		if (clientInputBuffer[i].TryGetInputPacket(tick, input) == true)
		{
			player->Update(elapsedTime, input);
		}
		else
		{
			player->Update(elapsedTime, player->GetLastInput());
		}
	}
}

void SimulationServer::ProcessDeactivation()
{
	while (!trashCan.empty())
	{
		trashCan.front()->Deactivate();
		trashCan.pop();
	}
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


void SimulationServer::SpawnItem(const float elapsedTime)
{
	itemSpawnTime += elapsedTime;
	if (itemSpawnTime <= itemRespawnTime)
	{
		return;
	}

	itemSpawnTime -= itemRespawnTime;
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

void SimulationServer::AccessNewPlayer(int playerNum)
{
	CPlayer* player = m_player[playerNum];
	player->SetPosition(initialPos[playerNum]);
	player->RotatePYR(initialRot[playerNum]);
	player->SetActive(true);
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
	auto prevIt = SnapshotLogMap.find(tick - 1);
	if (prevIt == SnapshotLogMap.end())
	{
		return;
	}

	const ServerSnapshot& prevSnapshot = prevIt->second;

	for (int playerIndex = 0; playerIndex < Protocol::kMaxPlayerCount; ++playerIndex)
	{
		for (int missileIndex = 0; missileIndex < Protocol::kMaxMissileCountPerPlayer; ++missileIndex)
		{
			CMissileObject* missile = m_player[playerIndex]->m_pMissiles[missileIndex];
			int missileSnapshotIndex = playerIndex * Protocol::kMaxMissileCountPerPlayer + missileIndex;

			bool prevMissileActive = prevSnapshot.missileSnapshots[missileSnapshotIndex].active;
			bool curMissileActive = missile->IsActive();

			if (prevMissileActive == curMissileActive)
			{
				continue;
			}

			eventQueue.PushPacket<LocalMissileEventPacket>(
				{
					CS_LocalMissileEvent,
					missile->GetID(),
					tick,
					playerIndex,
					curMissileActive
				}
			);
		}
	}
}

void SimulationServer::SaveLatestSnapshot()
{
	TickSnapshotPacket tickSnapshot{};
	tickSnapshot.serverTick = GetTick();
	for (int playerNum = 0; playerNum < Protocol::kMaxPlayerCount; ++playerNum)
	{
		CPlayer* player = m_player[playerNum];
		tickSnapshot.playerInfos[playerNum] = { playerNum, player->m_nHp, player->GetCurPos(), player->GetCurRot(), player->IsActive() };
		for (int missileNum = 0; missileNum < Protocol::kMaxMissileCountPerPlayer; ++missileNum)
		{
			CMissileObject* missile = player->m_pMissiles[missileNum];
			MissileInfoPacket& missileInfo = tickSnapshot.missileInfos[playerNum * Protocol::kMaxMissileCountPerPlayer + missileNum];
			missileInfo.position = missile->GetCurPos();
			missileInfo.active = missile->IsActive();
		}
	}

	for (int i = 0; i < Protocol::kMaxItemCount; ++i)
	{
		CItemObject* item = m_ItemObject[i];
		ItemInfoPacket& itemInfo = tickSnapshot.itemInfos[i];
		itemInfo.active = item->IsActive();
	}

	snapshotPacketBuffer.PushSnapshotPacket(tickSnapshot);
}

void SimulationServer::CleanSnapshotLogs()
{
	uint64_t curTick = GetTick();
	if (curTick > Protocol::kMaxRollbackTicks) {
		SnapshotLogMap.erase(curTick - Protocol::kMaxRollbackTicks);
	}
}
