#include"stdafx.h"
#include "Client.h"
char* CilentName = (char*)"test.mp4";
char* SERVERIP = (char*)"127.0.0.1";
struct headerFile {
	long long sendSize;
	int TitleLength;
};
#pragma pack()


ConnectServer::ConnectServer()
{

}

ConnectServer::~ConnectServer()
{
}

void ConnectServer::SetClientsock()
{
	int retval;
	<<<<<< < HEAD

		WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return;
	// ���� ����
	Clientsock = socket(AF_INET, SOCK_STREAM, 0);
	if (Clientsock == INVALID_SOCKET) err_quit("socket()");
	// 서버 커넥트
	====== =
		char* SERVERIP = (char*)"127.0.0.1";
	// ����� �μ��� ������ IP �ּҷ� ���
	//if (argc > 1) SERVERIP = argv[1];
	//int ServerPort[2] = {};
	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	// ���� ����
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");
	// connect()
	>>>>>> > c230ef1654f0abf4094088af84eba6f0d3cc0fea
		struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	<<<<<< < HEAD
		retval = connect(Clientsock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR)err_quit("socket()");
	====== =
		retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	//if (retval == SOCKET_ERROR)err_quit("socket()");
	// ������ ��ſ� ����� ����
	closesocket(sock);
	>>>>>> > c230ef1654f0abf4094088af84eba6f0d3cc0fea
}

void ConnectServer::SendtoServer(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}

void ConnectServer::SendtoServer(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'W': SendKey << 0x01;			break;							//0000 0001 
		case 'S': SendKey << 0x02;		    break;							//0000 0010
		case 'A': SendKey << 0x02;		    break;							//0000 0100
		case 'D': SendKey << 0x02;		    break;							//0000 1000
		case 'Q': SendKey << 0x02;		    break;							//0001 0000
		case 'R': SendKey << 0x02;		    break;							//0010 0000
		}
		char buf[BUFSIZE];
		char a = char(wParam);

	}
