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


PlayerInfoPacket SendPlayerDater[4];
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
	//if (retval == SOCKET_ERROR)err_quit("socket()");
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

DWORD __stdcall ConnectServer::ReceiveFromServer(LPVOID arg)
{
	int recvData{};
	int retval{};
	SOCKET client_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char PacketType;
	char PlayerNumber;
	XMFLOAT3  RotationAxis;
	XMFLOAT3 Rotation;
	addrlen = sizeof(clientaddr);
	PlayerInfoPacket PlayerInfo[4];
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	PlayerInfoPacket Client;
	while (1) {
		for (int i = 0; i < 4; i++) {
			retval = recv(client_sock, (char*)&PlayerInfo[i].packetType, sizeof(char), MSG_WAITALL);
			recvData += retval;
			retval = recv(client_sock, (char*)&PlayerInfo[i].playerNumber, sizeof(int), MSG_WAITALL);
			recvData += retval;
			retval = recv(client_sock, (char*)&PlayerInfo[i].movement, sizeof(XMFLOAT3), MSG_WAITALL);
			recvData += retval;
			retval = recv(client_sock, (char*)&PlayerInfo[i].rotation, sizeof(XMFLOAT3), MSG_WAITALL);
			recvData += retval;
			SendPlayerDater[i] = PlayerInfo[i];//->Player and otherPlayer render ->goto Scene.cpp render() and Player.cpp render()
			recvData = 0;
		}
		//break;->break x ->infinite run
	}
	//	printf("클라이언트 번호 : %d \n", Clientnum);
//	closesocket(client_sock);

//	while (1) {};

}
