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

#include "FrameDataManager.h"
#include "PacketCombiner.h"
#include "error.h"
#include "Missileobject.h"

#pragma comment(lib, "ws2_32")

#define SERVERPORT 9000
#define BUFSIZE 1024

class Client {
public:
	Client();
	Client(NetworkSyncManager* networkSyncMgr, FrameDataManager* frameDataMgr);
	~Client();

	SOCKET* GetClientsock() { return sock; }
	void ConnectServer();
	const char* GetServerIp() { return serverIp; }
	void KeyDownHandler(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void KeyUpHandler(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void PrepareInputPacket(XMFLOAT3& playerPYR);
	void GetKeyPacketToSend(PlayerKeyPacket& keyPacket);

	void PacketProcessHelper(char packetType, char* fillTarget);
	void ReceivePingPongPacket(const PingpongPacket& ppPacket);

	inline int GetPlayerNum() { return initData.playerNum; }


	
	FPoint deltaMouse;
	InitDataPacket initData;

	HANDLE FrameAdvanced;

	char remainBuffer[BUFSIZE]{};

	//Add
	XMFLOAT3  lastLaunchedMissilePos;
	uint64_t lastLaunchedMissileNum = 0;
	unsigned char sendKey = NULL;
	unsigned char prevKey = NULL;
	mutex inputPacketLock;
	condition_variable inputChangedCV;


	//Latency Interpolation
	PacketCombiner* packetCombiner;
	FrameDataManager* frameDataManager;
	NetworkSyncManager* networkSyncMgr;

private:
	SOCKET* sock = nullptr;

	const char* serverIp = (char*)"172.30.1.26";

	unsigned char option0 = 0x01;   // 0000 00c01 
	unsigned char option1 = 0x02;   // 0000 0010
	unsigned char option2 = 0x04;   // 0000 0100
	unsigned char option3 = 0x08;   // 0000 1000
	unsigned char option4 = 0x10;   // 0001 0000
	unsigned char option5 = 0x20;   // 0010 0000
	unsigned char option6 = 0x40;   // 0100 0000
	unsigned char option7 = 0x80;   // 1000 0000


	deque<PlayerKeyPacket> inputPacket_dq;
};

DWORD WINAPI ReceiveFromServer(LPVOID arg);
DWORD WINAPI SendPingPacket(LPVOID arg);
DWORD WINAPI SendInputPacket(LPVOID arg);

