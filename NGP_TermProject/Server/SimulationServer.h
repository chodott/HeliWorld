#pragma once

#include "GameObject.h"
#include "ClientInputBuffer.h"
#include "SnapShot.h"

#include <queue>

#define MAX_BOUNDARY_X		1000
#define MIN_BOUNDARY_X		0
#define MAX_BOUNDARY_Y		1000
#define MIN_BOUNDARY_Y		300
#define MAX_BOUNDARY_Z		1000
#define MIN_BOUNDARY_Z		0

#define RESPAWN_TIME		5.f

class SimulationServer
{
private:
	void CheckCollision();
	void SpawnItem();

	void ResetToSnapshot(uint64_t targetTick);
	void UpdateSnapshot(uint64_t targetTick);

	ClientInputBuffer& clientInputBuffer;


	unordered_map<uint64_t, ServerSnapshot> SnapshotLogMap;
	CItemObject* m_ItemObject[Protocol::kMaxItemCount];

	std::queue<GameObject*> trashCan;

	const float itemRespawnTime = 8.f;
	float itemSpawnTime = 0.f;
	float elapsedTime = 0.f;

	uint64_t serverTick = 0;

public:
	SimulationServer(ClientInputBuffer& inputBuffer);
	~SimulationServer();

	void Update();
	inline uint64_t GetTick() { return serverTick; }
};

