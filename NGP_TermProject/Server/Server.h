#pragma once

#include "Socket.h"
#include "SCPacket.h"
#include "GameObject.h"

#include <array>
#include <chrono>
#include <queue>


#define SERVERPORT		9000
#define BUFSIZE			512

DWORD WINAPI ReceiveFromClient(LPVOID arg);
DWORD WINAPI AcceptClient(LPVOID arg);

class Client;
class CPlayer;
class CItemObject;


class Clock {
public:
	using time = std::chrono::high_resolution_clock;
	using second = std::chrono::duration<float>;

	Clock()
		: timePassed(0.f), timeStamp(time::now()) {}

	void Record()
	{
		timeStamp = time::now();
	}

	float GetTimePassedFromLastUpdate()
	{
		return second(time::now() - timeStamp).count();
	}

private:
	float timePassed;
	time::time_point timeStamp;

};

class Server {
public:
	Server();
	~Server();

	void OpenListenSocket();

	void SendAllClient();

	// 클라 충돌 및 움직임 송수신
	void Update();
	void CheckCollision();
	void SpawnItem();

	SOCKET* GetSocket() { return &listenSock; }


	Clock timer;
	float itemSpawnTime = 0.f;
	float elapsedTime = 0.f;


	std::array<Client*, 4> clients;
	XMFLOAT3 initialPos[4]{
		{100,100,100},{150,100,100},{200,100,100},{250,100,100}
	};

	int connectedClients = 0;
	HANDLE updateDone;

	CItemObject* m_ItemObject[10];
	std::queue<GameObject*> trashCan;
private:
	SOCKET listenSock;

};


class Client {
public:
	Client();
	~Client();
	SOCKET sock;

	void SetPlayerNumber(int playerNumber) { m_playerNumber = (char)playerNumber; }
	int GetPlayerNumber() { return m_playerNumber; }

	void ToggleConnected() { m_connected = true; shouldDisconnected = false; }
	bool IsConnected() { return m_connected; }

	bool ShouldDisconnected() { return shouldDisconnected; }
	void Disconnect() { m_connected = false; shouldDisconnected = false; }

	void Reset();

	CPlayer* m_player = nullptr;

	float deadTime = 0.f;

private:
	int m_playerNumber = -1;	// maybe client class can have playerID inside

	bool m_connected = false;
	bool shouldDisconnected = false;

};

