#pragma once
#pragma warning(disable : 26495)

#include "Socket.h"
#include "SCPacket.h"
#include <array>


#define SERVERPORT		9000
#define BUFSIZE			512

const char KEY_W		= 0x01;
const char KEY_A		= 0x02;
const char KEY_S		= 0x04;
const char MOUSE_LEFT	= 0x10;
const char MOUSE_RIGHT	= 0x20;

DWORD WINAPI ReceiveAllClient(LPVOID arg);
DWORD WINAPI AcceptClient(LPVOID arg);

class Client;
class CPlayer;

class Server {
public:
	Server();
	~Server();

	void OpenListenSocket();

	void SendAllClient();

	// 클라 충돌 및 움직임 송수신
	void Update();
	void CheckCollision();


	SOCKET* GetSocket() { return &listenSock; }


	std::array<Client*, 4> clients;
	XMFLOAT3 initialPos[4]{
		{0,0,0},{50,0,0},{100,0,0},{150,0,0}
	};

private:
	SOCKET listenSock;

	std::array<PlayerInfoPacket, 4> infoPackets;

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

