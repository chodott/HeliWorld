#pragma once

#include "SCPacket.h"
#include "error.h"
#include "GameObject.h"
#include "ProtocolConstants.h"
#include "ClientInputBuffer.h"
#include "SnapshotPacketBuffer.h"
#include "Client.h"
#include "ServerContext.h"
#include "NetworkEventQueue.h"
#include "SnapshotPacketBuffer.h"

#include <array>
#include <chrono>
#include <queue>
#include <unordered_map>
#include <mutex>

#define  SERVERPORT 9000

class Clock {
public:
	using time = std::chrono::high_resolution_clock;
	using second = std::chrono::duration<float>;

	Clock()	: timePassed(0.f), timeStamp(time::now()) {}
	void Record() { timeStamp = time::now(); }
	float GetTimePassedFromLastUpdate() { return second(time::now() - timeStamp).count(); }
private:
	float timePassed;
	time::time_point timeStamp;
};

class NetworkServer {
public:
	NetworkServer(ClientInputBuffer& inputBuffer, SnapshotPacketBuffer& packetBuffer, NetworkEventQueue& networkEventQueue);
	~NetworkServer();

	void OpenListenSocket(ServerContext* serverContext);
	void CloseListenSocket();

	void SendPacketAllClient();
	atomic<bool> IsRunning() { return isRunning.load(); }

	uint64_t GetTimestampMs();

	SOCKET* GetSocket() { return &listenSock; }

	Clock timer;

	std::array<Client*, Protocol::kMaxPlayerCount> clients;

	int connectedClients = 0;

private:
	HANDLE acceptHandle = nullptr;
	HANDLE sendHandle = nullptr;

	atomic<bool> isRunning = false;
	SOCKET listenSock;
	ServerContext serverContext;

	thread acceptThread;
	thread sendThread;

	ClientInputBuffer& clientInputBuffer;
	SnapshotPacketBuffer& snapshotPacketBuffer;
	NetworkEventQueue& eventQueue;
};

