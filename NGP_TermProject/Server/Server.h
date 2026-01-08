#pragma once

#include "Socket.h"
#include "SCPacket.h"
#include "GameObject.h"
#include "Snapshot.h"
#include "ProtocolConstants.h"
#include "ClientInputBuffer.h"
#include "SimulationServer.h"

#include <concurrent_queue.h>
#include <array>
#include <chrono>
#include <queue>
#include <unordered_map>
#include <mutex>

#define SERVERPORT		9000
#define BUFSIZE			512

#define MAP_SCALE 32.767

DWORD WINAPI ReceiveFromClient(LPVOID arg);
DWORD WINAPI AcceptClient(LPVOID arg);


class Client;
class CPlayer;
class CItemObject;

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

class Server {
public:
	Server();
	~Server();

	void OpenListenSocket();

	void SendPacketAllClient();

	PlayerKeyPacket keyPackets[Protocol::kMaxPlayerCount];

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
};


class Client {
public:
	Client();
	~Client();
	SOCKET sock;

	void SetPlayerNumber(int playerNumber) { m_playerNumber = (char)playerNumber; }
	int GetPlayerNumber() { return m_playerNumber; }

	void Connected() { m_connected = true; shouldDisconnected = false; }
	bool IsConnected() { return m_connected; }

	bool ShouldDisconnected() { return shouldDisconnected; }
	void Disconnect() { m_connected = false; shouldDisconnected = false; }
	bool ShouldSendEvent(uint64_t id);

	void Reset();

	CPlayer* m_player = nullptr;


	//Check RTT
	char remainBuffer[512]{};
	int receivedBytes = 0;
	int remainSize = 0;
	
	//Latency
	concurrency::concurrent_queue<PlayerKeyPacket> keyPacket_q;
	
	float deadTime = 0.f;
private:
	uint64_t lastLaunchedMissileID = 0;
	int m_playerNumber = -1;	// maybe client class can have playerID inside

	bool m_connected = false;
	bool shouldDisconnected = false;
};


DWORD WINAPI SendAllClient(LPVOID arg);

