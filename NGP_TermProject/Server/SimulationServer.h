#pragma once

#include "ProtocolConstants.h"
#include "GameObject.h"
#include "ClientInputBuffer.h"
#include "SnapShot.h"
#include "SnapshotPacketBuffer.h"
#include "NetworkEventQueue.h"
#include "InputManager.h"
#include "TickRingBuffer.h"


#include <queue>
#include <array>
#include <atomic>

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

	bool ResetToSnapshot(uint64_t targetTick);
	void UpdateSnapshot(uint64_t targetTick);
	void GenerateMissileEvents(uint64_t tick);

	void SaveLatestSnapshot();

	static XMFLOAT3 initialPos[Protocol::kMaxPlayerCount];
	static XMFLOAT3 initialRot[Protocol::kMaxPlayerCount];

	InputManager& inputManager;
	SnapshotPacketBuffer& snapshotPacketBuffer;
	NetworkEventQueue& eventQueue;

	// 완료 틱 T 기준 [T-R, T] 상태를 보관한다(R=kRollbackInputTicks, 현재 6).
	// 첫 재실행 직전 상태까지 총 R+1칸이 필요하며 만료된 칸은 자동으로 덮어쓴다.
	TickRingBuffer<ServerSnapshot, Protocol::kSnapshotHistoryCapacity> snapshotHistory;
	CPlayer* m_player[Protocol::kMaxPlayerCount];
	CItemObject* m_ItemObject[Protocol::kMaxItemCount];

	std::queue<GameObject*> trashCan;

	const float itemRespawnTime = 8.f;
	float itemSpawnTime = 0.f;
	// 서버가 마지막으로 처리를 완료한 틱 T. 0은 최초 상태이고 다음 실행 틱은 T+1이다.
	// 네트워크 스레드도 입력 범위 검증에 읽으므로 원자적으로 접근한다.
	std::atomic<uint64_t> serverTick{ 0 };

public:
	SimulationServer(InputManager& inputMgr, SnapshotPacketBuffer& packetBuffer, NetworkEventQueue& networkEventQueue);
	~SimulationServer();

	void AccessNewPlayer(int playerNum);

	void Update(const float elapsedTime);
	inline uint64_t GetTick() const { return serverTick.load(); }
};

