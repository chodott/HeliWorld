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

	for (int i = 0; i < 4; ++i)
	{
		//ego jonna byeollo - Chodot
		clients[i] = new Client;
		players[i] = new CPlayer;
	}
}

Server::~Server()
{
	WSACleanup();
	for (int i = 0; i < 4; ++i)
	{
		delete(clients[i]);
		delete(players[i]);
	}
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
		// sending playerinfo packet
		char packetType = SC_PlayerInfo;
		for (int j = 0; i < clientNum; ++i)
		{
			//if (clients.at(i) != nullptr)
				//send(clients.at(i)->clientSock, (char*)infoPackets.at(i), sizeof(PlayerInfoPacket), 0);
				// need conversion operator packet => char* 
			if (clients.at(i) != nullptr)
			{
				SOCKET* sock = &clients.at(i)->clientSock;
				Client* c = clients.at(i);
				XMFLOAT3 movement{  c->xPos - c->oldxPos,
									c->yPos - c->oldyPos,
									c->zPos - c->oldzPos };
				XMFLOAT3 rotation{  c->pitch - c->oldPitch,
									c->yaw - c->oldYaw,
									c->roll - c->oldRoll };
				send(*sock, (char*)&packetType, sizeof(char), 0);
				send(*sock, (char*)&i, sizeof(char), 0);
				send(*sock, (char*)&movement, sizeof(char), 0);
				send(*sock, (char*)&rotation, sizeof(char), 0);
			}
		}
	}
}

void Server::AnimateObjects()
{
}

void Server::CheckCollision()
{
	for(int i=0; i<4; ++i)
	{
		if (!players[i]->GetActive()) continue;

		for (int j = 0; j < 4; ++j)
		{
			if (i == j) continue;		//Same Player

			//Collision Players
			if (players[j]->GetActive() && players[i]->GetBoundingBox().Intersects(players[j]->GetBoundingBox()))
			{
				
			}

			for (auto& missile : players[j]->m_pMissiles)
			{
				//Collision between Player and Missiles
				if (players[i]->GetBoundingBox().Intersects(missile.GetBoundingBox()))
				{
					players[i]->SetActive(FALSE);
					missile.SetActive(FALSE);
				}
			}
	
		}
	}

	/*if (pMissleObject && PlayerObject)
	{
		for (int i = 0; i < sizeof(PlayerObject); ++i)
		{
			for (int j = 0; j < sizeof(pMissleObject); ++j)
			{
				if (PlayerObject[i]->GetActive() && pMissleObject[j]->GetActive()) {
					if (PlayerObject[i]->Player_id != pMissleObject[j]->Player_id) {
						if (PlayerObject[i]->m_xmOOBB.Intersects(pMissleObject[j]->m_xmOOBB))
						{
							PlayerObject[i]->SetActive(FALSE);
							pMissleObject[j]->SetActive(FALSE);
							break;
						}
					}
				}
			}
		}
	}*/
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
			if (!g_server->clients.at(i)) {
				g_server->clients.at(i)->clientSock = Temp_sock;
				index = i;
				break;
			}
		}
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP���� ] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
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



	//while (1) {
	//	// Event off
	//	for (int i = 0; i < 4; ++i)
	//	{
	//		char buf[BUFSIZE];
	//		retval = recv(g_server->clients.at(i)->clientSock, buf, BUFSIZE, 0);
	//		g_server->playerKey[i] = buf[0];
	//	}
	//	if (retval == SOCKET_ERROR) {
	//		err_display("recv()");
	//		break;
	//	}

	//	int playerNumber = (int)buf[0];
	//	g_server->playerKey[playerNumber] = buf[1];
	//	// Event on
	//}

	return 1;
}

int main()
{
	//Create Object Mgr
	g_server = new Server;
	g_server->OpenListenSocket();
	HANDLE AcceptThread, ReceiveThread;
	AcceptThread = CreateThread(NULL, 0, AcceptClient, (LPVOID)&g_server->clients, 0, NULL);//client->g_server
	ReceiveThread = CreateThread(NULL, 0, ReceiveAllClient, (LPVOID)&g_server->clients, 0, NULL);
	while (1)
	{
		// Event is on?
		g_server->AnimateObjects();
		g_server->CheckCollision();
		//SendAllClient
	}
}





