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

typedef std::chrono::steady_clock sclock;
class Timer {
public:
	virtual ~Timer() { }
	inline void start() { start_time = sclock::now(); }
	inline void stop() { end_time = sclock::now(); }

	inline double elapsed_milli() const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
	}

	inline double elapsed_sec() const
	{
		return elapsed_milli() / 1000.0;
	}

	sclock::time_point start_time;
	sclock::time_point end_time;
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
	Timer timerHandle;

	std::array<Client*, 4> clients;
	XMFLOAT3 initialPos[4]{
		{100,100,100},{150,100,100},{200,100,100},{250,100,100}
	};

private:
	SOCKET listenSock;

	std::array<PlayerInfoPacket, 4> infoPackets;
	CItemObject m_ItemObject[10];

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

	int m_playerNumber;	// maybe client class can have playerID inside

	int hp = 70;

	bool m_connected = false;

};

