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

	SOCKET* GetClientsock() { return sock; };
	void ConnectServer();
	char* GetSERVERIP() { return SERVERIP; };
	void KeyDownHandler(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void KeyUpHandler(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void SendtoServer();
	PlayerInfoPacket playerData[4];
	PlayerStatusPacket playerStatus[4];
	MissileInfoPacket missilePacket[32];
	ItemInfoPacket itemPacket[10];
	FPoint deltaMouse;

	int PlayerNum = 0;

	HANDLE ReceiveDone;

	// JJam Tong
	char remain[512]{};
	int remainOffset = 0;
	int remainSize = 0;





private:

	SOCKET* sock = nullptr;

	//char* SERVERIP = (char*)"192.168.203.20";
//	char* SERVERIP = (char*)"127.0.0.1";
	char* SERVERIP = (char*)"127.0.0.1";
	unsigned char sendKey = NULL;
	unsigned char option0 = 0x01;	// 0000 0001 
	unsigned char option1 = 0x02;	// 0000 0010
	unsigned char option2 = 0x04;	// 0000 0100
	unsigned char option3 = 0x08;	// 0000 1000
	unsigned char option4 = 0x10;	// 0001 0000
	unsigned char option5 = 0x20;	// 0010 0000
	unsigned char option6 = 0x40;	// 0100 0000
	unsigned char option7 = 0x80;	// 1000 0000
};

DWORD WINAPI ReceiveFromServer(LPVOID arg);

