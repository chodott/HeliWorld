#pragma once

#include "Snapshot.h"
#include "NetworkServer.h"
#include "SimulationServer.h"
#include "ServerContext.h"

#include <concurrent_queue.h>

#define BUFSIZE 512

class Client {
public:
	Client() {}
	~Client() {}

	void SetPlayerNumber(int playerNumber) { m_playerNumber = (char)playerNumber; }
	int GetPlayerNumber() { return m_playerNumber; }

	void Connect(SOCKET clientSock, int playerNum, ServerContext* serverContext);
	bool IsConnected() { return m_connected; }

	bool ShouldDisconnected() { return shouldDisconnected; }
	void Disconnect() { m_connected = false; shouldDisconnected = false; }
	bool ShouldSendEvent(uint64_t id);

	void Reset();

	inline SOCKET GetSocket() { return sock; }

	template<typename T>
	inline void SendPacket(const T& packet) { send(sock, reinterpret_cast<const char*>(&packet), sizeof(T), 0); }

	//Check RTT
	char remainBuffer[512]{};
	int receivedBytes = 0;
	int remainSize = 0;

	//Latency

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