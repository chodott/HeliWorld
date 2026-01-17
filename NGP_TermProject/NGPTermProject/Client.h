#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <winsock2.h>
#include <ws2tcpip.h>

#include <tchar.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <fcntl.h>
#include <algorithm>
#include <queue>
#include <concurrent_queue.h>
#include <mutex>
#include <thread>

#include "FrameDataManager.h"
#include "error.h"
#include "Missileobject.h"

#pragma comment(lib, "ws2_32")

#define SERVERPORT 9000
#define BUFSIZE 4096

class Client {
public:
	Client();
	~Client();

	SOCKET* GetClientsock() { return sock; }
	void ConnectServer(InitDataPacket& initData);
	const char* GetServerIp() { return serverIp; }
	void GetKeyPacketToSend(PlayerKeyPacket& keyPacket);
	void AddKeyPacket(const PlayerKeyPacket& keyPacket);

	void SendPingPacket(PingpongPacket& keyPacket);

	void PacketProcessHelper(char packetType, char* fillTarget);

	inline void RegisterListener(IPacketListener* packetListener) { packetListner_vec.push_back(packetListener); }

	
	HANDLE FrameAdvanced;

	char remainBuffer[BUFSIZE]{};

	mutex inputPacketLock;
	condition_variable inputChangedCV;

private:
	SOCKET* sock = nullptr;

	std::thread recvThread;
	std::thread sendInputThread;

	const char* serverIp = (char*)"127.0.0.1";

	vector<IPacketListener*> packetListner_vec;

	deque<PlayerKeyPacket> inputPacket_dq;
	concurrency::concurrent_queue<PingpongPacket> pingpongPacket_q;
};

