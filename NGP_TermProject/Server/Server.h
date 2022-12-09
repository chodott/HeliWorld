#pragma once
#pragma warning(disable : 26495)

#include "Socket.h"
#include "SCPacket.h"
#include <array>
#include"GameObject.h"
#include <queue>


#define SERVERPORT		9000
#define BUFSIZE			512

DWORD WINAPI ReceiveAllClient(LPVOID arg);
DWORD WINAPI AcceptClient(LPVOID arg);

class Client;
class CPlayer;
class CItemObject;
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
		{100,100,100},{150,100,100},{200,100,100},{250,100,100}
	};

	std::queue<GameObject*> trashCan;
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

