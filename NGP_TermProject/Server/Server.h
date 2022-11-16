#pragma once
//#include "error.h"
#include <unordered_map>
#include <vector>
#include "Socket.h"
#include "SCPacket.h"
#include <array>

#include "Socket.h"
#include "GameObjectMgr.h"
#define SERVERPORT 9000
#define BUFSIZE    512


const char KEY_W = 0x01;
const char KEY_A = 0x02;
const char KEY_S = 0x04;
const char MOUSE_LEFT = 0x10;
const char MOUSE_RIGHT = 0x20;

int RecieveServer();

DWORD WINAPI ReceiveAllClient(LPVOID arg);

int ReceiveServer();

class Client;

class Server {
public:
	Server();
	~Server();

	void OpenListenSocket();

	void SendAllClient();

	SOCKET* GetSocket() { return &listenSock; }

private:
	SOCKET listenSock;

	int clientNum = 0;
	std::array<Client*, 4> clients;

	std::array<PlayerInfoPacket, 4> infoPackets;

};


class Client {
public:
	friend Server;

private:
	SOCKET clientSock;

	//char playerNumber;	// maybe client class don't need it inside
	
	float oldxPos, oldyPos, oldzPos;
	float xPos, yPos, zPos;

	float oldPitch, oldYaw, oldRoll;
	float pitch, yaw, roll;

	int hp;

};

Server g_server;
