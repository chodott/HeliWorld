#pragma once
//#include "error.h"
#include "Socket.h"
#include "SCPacket.h"
#include <array>

#define SERVERPORT 9000
#define BUFSIZE    512

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
