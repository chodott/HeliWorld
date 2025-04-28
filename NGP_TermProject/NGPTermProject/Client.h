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
#include <chrono>
#include <algorithm>
#include <queue>
#include <deque>

#include "CSPacket.h"
#include "error.h"
#include "Missileobject.h"

#pragma comment(lib, "ws2_32")

#define SERVERPORT 9000
#define BUFSIZE 512

class Client {
public:
	Client();
	~Client();

	SOCKET* GetClientsock() { return sock; }
	void ConnectServer();
	char* GetServerIp() { return serverIp; }
	void KeyDownHandler(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void KeyUpHandler(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void SendtoServer();

	uint32_t GetTimestampMs();

	PlayerInfoPacket playerData[4];
	MissileInfoPacket missilePacket[32];
	ItemInfoPacket itemPacket[10];
	FPoint deltaMouse;

	int PlayerNum = 0;

	HANDLE FrameAdvanced;

	char remain[512]{};
	int remainOffset = 0;
	int remainSize = 0;

	int16_t lastLaunchedMissileNum = -1;
	unsigned char sendKey = NULL;

private:
	SOCKET* sock = nullptr;

	char* serverIp = (char*)"172.30.1.66";

	unsigned char option0 = 0x01;   // 0000 0001 
	unsigned char option1 = 0x02;   // 0000 0010
	unsigned char option2 = 0x04;   // 0000 0100
	unsigned char option3 = 0x08;   // 0000 1000
	unsigned char option4 = 0x10;   // 0001 0000
	unsigned char option5 = 0x20;   // 0010 0000
	unsigned char option6 = 0x40;   // 0100 0000
	unsigned char option7 = 0x80;   // 1000 0000
};

DWORD WINAPI ReceiveFromServer(LPVOID arg);
