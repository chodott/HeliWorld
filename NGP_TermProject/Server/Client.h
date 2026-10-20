#pragma once

#include "Snapshot.h"
#include "NetworkServer.h"
#include "SimulationServer.h"
#include "ServerContext.h"

#include <thread>
#include <atomic>
#include <mutex>
#include <concurrent_queue.h>

class Client {
public:
	Client() {}
	~Client();

	void SetPlayerNumber(int playerNumber) { m_playerNumber = playerNumber; }
	int GetPlayerNumber() { return m_playerNumber; }

	void Connect(SOCKET clientSock, int playerNum, ServerContext* serverContext);
	bool IsConnected() const { return m_connected.load() && !shouldDisconnected.load(); }

	void RequestDisconnect();
	void CleanupDisconnected();
	void Disconnect();
	bool ShouldSendEvent(uint64_t id);

	inline SOCKET GetSocket() const { return sock.load(); }

	template<typename T>
	bool SendPacket(const T& packet) { return SendBytes(reinterpret_cast<const char*>(&packet), sizeof(T)); }

	//Latency

	float deadTime = 0.f;
private:
	void CloseConnection(); // Caller holds connectionLock; never called by recvThread.
	bool SendBytes(const char* data, int size);
	std::mutex connectionLock;
	std::mutex sendLock;
	std::atomic<SOCKET> sock{ INVALID_SOCKET };
	std::thread recvThread;
	ReceiveClientContext recvCtx{};

	uint64_t lastLaunchedMissileID = 0;
	int m_playerNumber = -1;	// maybe client class can have playerID inside

	std::atomic<bool> m_connected{ false };
	std::atomic<bool> shouldDisconnected{ false };
};
