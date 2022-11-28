#pragma once
#include "Socket.h"
#include "SCPacket.h"
#include <array>


#define SERVERPORT		9000
#define BUFSIZE			512

const char KEY_W = 0x01;
const char KEY_A = 0x02;
const char KEY_S = 0x04;
const char MOUSE_LEFT = 0x10;
const char MOUSE_RIGHT = 0x20;

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
	//std::array<CPlayer*, 4> players;
	//std::array<char, 4> playerKey;
	//std::array<char, 4> playerMouse;
	float initialPos[4][3]{
		{500,0,300},{1000,500,300},{500,1000,300},{0,500,300}
	};

	HANDLE ReceiveEvent;

private:
	SOCKET listenSock;


	std::array<PlayerInfoPacket, 4> infoPackets;

};


class Client {
public:
	Client();
	SOCKET sock;
	float oldxPos = 0.f, oldyPos = 0.f, oldzPos = 0.f;
	float xPos = 0.f, yPos = 0.f, zPos = 0.f;

	float oldPitch = 0.f, oldYaw = 0.f, oldRoll = 0.f;
	float pitch = 0.f, yaw = 0.f, roll = 0.f;

	void SetPlayerNumber(int playerNumber) { m_playerNumber = (char)playerNumber; }

	void ToggleConnected() { m_connected = !m_connected; }
	bool IsConnected() { return m_connected; }

	CPlayer* m_player = nullptr;
private:

	char m_playerNumber;	// maybe client class can have playerID inside

	int hp = 70;

	bool m_connected = false;

};

