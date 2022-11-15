#include"stdafx.h"

#define _CRT_SECURE_NO_WARNINGS // 구형 C 함수 사용 시 경고 끄기
#define _WINSOCK_DEPRECATED_NO_WARNINGS // 구형 소켓 API 사용 시 경고 끄기

#include <winsock2.h> // 윈속2 메인 헤더
#include <ws2tcpip.h> // 윈속2 확장 헤더

#include <tchar.h> // _T(), ...
#include <stdio.h> // printf(), ...
#include <stdlib.h> // exit(), ...
#include <string.h> // strncpy(), ...
#include <string>
#include <iostream>

#include"error.h"

#pragma comment(lib, "ws2_32") // ws2_32.lib 링크
#define SERVERPORT 9000
#define BUFSIZE 512

#pragma pack(1)
#pragma once

class ConnectServer {
public:
	ConnectServer();
	~ConnectServer();

	SOCKET GetClientsock() { return Clientsock; };
	void SetClientsock();
	char* GetSERVERIP() { return SERVERIP; };
	void SendtoServer(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
private:
	SOCKET Clientsock = 0;
	char* SERVERIP = (char*)"127.0.0.1";
	WSADATA wsa;
	int retval;
	struct sockaddr_in serveraddr;
	unsigned char SendKey;
	unsigned char option0 = 1 << 0; // 0000 0001 
	unsigned char option1 = 1 << 1; // 0000 0010
	unsigned char option2 = 1 << 2; // 0000 0100
	unsigned char option3 = 1 << 3; // 0000 1000
	unsigned char option4 = 1 << 4; // 0001 0000
	unsigned char option5 = 1 << 5; // 0010 0000
	unsigned char option6 = 1 << 6; // 0100 0000
	unsigned char option7 = 1 << 7; // 1000 0000
};
//SOCKET ConnectServer();
