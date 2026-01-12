#pragma once

#include "SCPacket.h"
#include "GameObject.h"
#include "ProtocolConstants.h"
#include "ClientInputBuffer.h"
#include "SimulationServer.h"
#include "SnapshotPacketBuffer.h"
#include "Client.h"
#include "ServerContext.h"

#include <array>
#include <chrono>
#include <queue>
#include <unordered_map>
#include <mutex>

#define SERVERPORT		9000
#define BUFSIZE			512

#define MAP_SCALE 32.767

DWORD WINAPI ReceiveFromClient(LPVOID arg);


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
	NetworkServer(ClientInputBuffer& inputBuffer, SnapshotPacketBuffer& packetBuffer);
	~NetworkServer();

	void OpenListenSocket(ServerContext* serverContext);
	void CloseListenSocket();

	void SendPacketAllClient();

	void PreparePackets();
	void GenerateEvents(uint64_t tick);

	uint64_t GetTimestampMs();

	SOCKET* GetSocket() { return &listenSock; }

	Clock timer;

	std::array<Client*, Protocol::kMaxPlayerCount> clients;

	int connectedClients = 0;
	HANDLE updateDone;


	template <typename T>
	inline void PushPacket(const T& packet){GetQueue<T>().push(packet);}
	template <typename T>
	inline bool TryPopPacket(T& outPacket){return GetQueue<T>().try_pop(outPacket);}
	template<typename T>
	inline void SendPacket(SOCKET& recvSocket, const T& packet) { send(recvSocket, reinterpret_cast<const char*>(&packet), sizeof(T), 0); }


private:
	template <typename T>
	static concurrency::concurrent_queue<T>& GetQueue()
	{
		static concurrency::concurrent_queue<T> queue;
		return queue;
	}

	SOCKET listenSock;
	ServerContext serverContext;

	HANDLE acceptHandle;
	HANDLE sendHandle;

	ClientInputBuffer& clientInputBuffer;
	SnapshotPacketBuffer& snapshotPacketBuffer;
};

