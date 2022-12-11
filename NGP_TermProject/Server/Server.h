#pragma once
#pragma warning(disable : 26495)

#include "Socket.h"
#include "SCPacket.h"
#include"GameObject.h"

#include <array>
#include <chrono>
#include <thread>


#define SERVERPORT		9000
#define BUFSIZE			512

DWORD WINAPI ReceiveAllClient(LPVOID arg);
DWORD WINAPI AcceptClient(LPVOID arg);

class Client;
class CPlayer;
class CItemObject;

//class Clock
//{
//public:
//	using time = std::chrono::high_resolution_clock;
//	using second = std::chrono::duration<float>;
//
//	Clock() noexcept
//		: timePassed(0), timeStamp(time::now()) {}
//	void UpdateClock() noexcept
//	{
//		timePassed += GetTimeFromLastUpdate();
//		timeStamp = time::now();
//	}
//
//	float GetTimeFromLastUpdate() const noexcept
//	{
//		return std::chrono::duration<float>(time::now() - timeStamp).count();
//	}
//public:
//	float timePassed;
//private:
//	time::time_point timeStamp;
//};


class Clock {
public:
	using time = std::chrono::high_resolution_clock;
	using second = std::chrono::duration<float>;

	Clock()
		: timePassed(0), timeStamp(time::now()) {}

	float TimePassed()
	{
		timePassed += GetTimePassedFromLastUpdate();
		timeStamp = time::now();

		return timePassed;
	}

	void Reset()
	{
		timePassed = 0.f;
		timeStamp = time::now();
	}

private:
	float GetTimePassedFromLastUpdate() const
	{
		return second(time::now() - timeStamp).count();
	}

	float timePassed;
	time::time_point timeStamp;
};

#include <queue>
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
	Clock healItemTimer;

	std::array<Client*, 4> clients;
	XMFLOAT3 initialPos[4]{
		{100,100,100},{150,100,100},{200,100,100},{250,100,100}
	};

	CItemObject* m_ItemObject[10];
	std::queue<GameObject*> trashCan;
private:
	SOCKET listenSock;

};


class Client {
public:
	Client();
	SOCKET sock;

	void SetPlayerNumber(int playerNumber) { m_playerNumber = (char)playerNumber; }
	int GetPlayerNumber() { return m_playerNumber; }

	void ToggleConnected() { m_connected = true; }
	bool IsConnected() { return m_connected; }

	CPlayer* m_player = nullptr;

private:
	int m_playerNumber{-1};	// maybe client class can have playerID inside

	bool m_connected = false;

};

