//#include "stdafx.h"
//#include <WS2tcpip.h>
#include "Server.h"


//#include "Socket.h"

//#pragma lib(lib, "ws2_32")

std::unordered_map<SOCKET, int> PlayerDataMap;
//SOCKET client_sock[4];
Server* g_server;

Server::Server()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
}

Server::~Server()
{
	WSACleanup();
}

void Server::OpenListenSocket()
{
	// ���� ����
	listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	//retval = bind(listenSock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (bind(listenSock, (sockaddr*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
		err_quit("bind()");
	//if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR)
		err_quit("listen()");
	//if (retval == SOCKET_ERROR) err_quit("listen()");
}

void Server::SendAllClient()
{
	for (int i = 0; i < clientNum; ++i)
	{
		for (int j = 0; i < clientNum; ++i)
		{
			//if (clients.at(i) != nullptr)
				//send(clients.at(i)->clientSock, (char*)infoPackets.at(i), sizeof(PlayerInfoPacket), 0);
				// need conversion operator packet => char* 
		}
	}
}

DWORD  AcceptClient(LPVOID arg)
{
	int index = 0;
	int retval;
	SOCKET Temp_sock = (SOCKET)arg;
	struct sockaddr_in clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];
	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		Temp_sock = accept(*g_server->GetSocket(), (struct sockaddr*)&clientaddr, &addrlen);
		if (Temp_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}
		for (int i = 0; i < 4; ++i) {
			if (!g_server->clientSock[i]) {
				g_server->clientSock[i] = Temp_sock;
				index = i;
				break;
			}
		}
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));
		//closesocket(client_sock[index]);
		printf("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}
	//closesocket(*g_server->GetSocket());
	//WSACleanup();
	return 0;
}

DWORD WINAPI ReceiveAllClient(LPVOID arg)//
{
	int retval{};
	sockaddr_in clientaddr{};
	char addr[INET_ADDRSTRLEN];
	char buf[BUFSIZE + 1];
	char PlayerInput[4];



	// Ŭ���̾�Ʈ�� ������ ���
	while (1) {
		// Event off
		for (int i = 0; i < 4; ++i)
		{
			char buf[BUFSIZE];
			retval = recv(g_server->clientSock[i], buf, BUFSIZE, 0);
			g_server->playerKey[i] = buf[0];
		}
		//retval = recv(g_server->clientSock[i], buf, BUFSIZE, 0);
		if (retval == SOCKET_ERROR) {
			err_display("recv()");
			break;
		}

		int playerNumber = (int)buf[0];
		g_server->playerKey[playerNumber] = buf[1];
		// Event on
	}

	return 1;
}

int main()
{
	//Create Object Mgr
	g_server = new Server;
	g_server->OpenListenSocket();
	GameObjectMgr* ObjectMgr = new GameObjectMgr();
	HANDLE AcceptThread, ReceiveThread;
	AcceptThread = CreateThread(NULL, 0, AcceptClient, (LPVOID)&g_server->clients, 0, NULL);//client->g_server
	ReceiveThread = CreateThread(NULL, 0, ReceiveAllClient, (LPVOID)&g_server->clients, 0, NULL);
	while (1)
	{
		// Event is on?
		ObjectMgr->AnimateObjects();
		ObjectMgr->CheckCollision();
		//SendAllClient
	}
}





