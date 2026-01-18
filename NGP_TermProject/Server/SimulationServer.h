#pragma once

#include "ProtocolConstants.h"
#include "GameObject.h"
#include "ClientInputBuffer.h"
#include "SnapShot.h"
#include "SnapshotPacketBuffer.h"
#include "NetworkEventQueue.h"

#include <queue>

#define MAX_BOUNDARY_X		1000
#define MIN_BOUNDARY_X		0
#define MAX_BOUNDARY_Y		1000
#define MIN_BOUNDARY_Y		300
#define MAX_BOUNDARY_Z		1000
#define MIN_BOUNDARY_Z		0

class SimulationServer
{
private:
	void ProcessPlayerInputs(const uint64_t tick, const float elapsedTime);
	void ProcessDeactivation();
	void CheckCollision();
	void SpawnItem(const float elapsedTime);

	void ResetToSnapshot(uint64_t targetTick);
	void UpdateSnapshot(uint64_t targetTick);
	void GenerateMissileEvents(uint64_t tick);

	void SaveLatestSnapshot();
	void CleanSnapshotLogs();

	static XMFLOAT3 initialPos[Protocol::kMaxPlayerCount];
	static XMFLOAT3 initialRot[Protocol::kMaxPlayerCount];

	ClientInputBuffer& clientInputBuffer;
	SnapshotPacketBuffer& snapshotPacketBuffer;
	NetworkEventQueue& eventQueue;

	unordered_map<uint64_t, ServerSnapshot> SnapshotLogMap;
	CPlayer* m_player[Protocol::kMaxPlayerCount];
	CItemObject* m_ItemObject[Protocol::kMaxItemCount];

	std::queue<GameObject*> trashCan;

	const float itemRespawnTime = 8.f;
	float itemSpawnTime = 0.f;
	uint64_t serverTick = 0;

public:
	SimulationServer(ClientInputBuffer& inputBuffer, SnapshotPacketBuffer& packetBuffer, NetworkEventQueue& networkEventQueue);
	~SimulationServer();

	void AccessNewPlayer(int playerNum);

	void Update(const float elapsedTime);
	inline uint64_t GetTick() { return serverTick; }
};

