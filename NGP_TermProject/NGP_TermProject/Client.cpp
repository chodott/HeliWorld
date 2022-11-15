#include"stdafx.h"
#include "Client.h"
char* CilentName = (char*)"test.mp4";
char* SERVERIP = (char*)"127.0.0.1";
struct headerFile {
	long long sendSize;
	int TitleLength;
};
#pragma pack()
#pragma once

ConnectServer::ConnectServer()
{

}

ConnectServer::~ConnectServer()
{
}

void ConnectServer::SetClientsock()
{
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return;
	Clientsock = socket(AF_INET, SOCK_STREAM, 0);
	if (Clientsock == INVALID_SOCKET) err_quit("socket()");
	// 서버 커넥트
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(Clientsock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)err_quit("socket()");
	//	closesocket(Clientsock);
}



void ConnectServer::SendtoServer(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'W': SendKey |= option1;	break;	//0000 0001 
		case 'S': SendKey |= option2;	break;	//0000 0010
		case 'A': SendKey |= option3;	break;	//0000 0100
		case 'D': SendKey |= option4;	break;	//0000 1000
		case 'Q': SendKey |= option5;	break;	//0001 0000
		case 'R': SendKey |= option6;	break;	//0010 0000
		}

	}
	char buf[1];
	memset(buf, '#', sizeof(buf));
	buf[0] = SendKey;
	retval = send(Clientsock, buf, 1, 0);
	if (retval == SOCKET_ERROR) {
		err_display("send()");
		return;
	}
	printf("[TCP 클라이언트] %d바이트를 보냈습니다.\n", retval);
}
