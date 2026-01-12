#pragma once

#include "Socket.h"
#include "Snapshot.h"
#include "Server.h"
#include "SimulationServer.h"
#include "ServerContext.h"

#include <concurrent_queue.h>


class Client {
public:
	Client() {}
	~Client() {}

	void SetPlayerNumber(int playerNumber) { m_playerNumber = (char)playerNumber; }
	int GetPlayerNumber() { return m_playerNumber; }

	void Connect(SOCKET clientSock, int playerNum, ServerContext* serverContext);
	void Connected() { m_connected = true; shouldDisconnected = false; }
	bool IsConnected() { return m_connected; }

	bool ShouldDisconnected() { return shouldDisconnected; }
	void Disconnect() { m_connected = false; shouldDisconnected = false; }
	bool ShouldSendEvent(uint64_t id);

	void Reset();

	inline SOCKET GetSocket() { return sock; }

	//Check RTT
	char remainBuffer[512]{};
	int receivedBytes = 0;
	int remainSize = 0;

	//Latency
	concurrency::concurrent_queue<PlayerKeyPacket> keyPacket_q;

	float deadTime = 0.f;
private:
	SOCKET sock;
	HANDLE recvHandle;
	ReceiveClientContext recvCtx{};

	uint64_t lastLaunchedMissileID = 0;
	int m_playerNumber = -1;	// maybe client class can have playerID inside

	bool m_connected = false;
	bool shouldDisconnected = false;
};