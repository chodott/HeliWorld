#include "Server.h"
#include "GameObject.h"


Server::Server()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	for (int i = 0; i < 4; ++i)
	{
		clients[i] = nullptr;
		//ego jonna byeollo - Chodot
		//clients[i] = new Client;
		//players[i] = new CPlayer;
	}
}

Server::~Server()
{
	WSACleanup();
	for (int i = 0; i < 4; ++i)
	{
		if (clients[i] != nullptr)
			delete(clients[i]);
		//delete(players[i]);
	}
}

void Server::OpenListenSocket()
{
	// create listen socket
	if ((listenSock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) err_quit("socket()");

	sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	if (bind(listenSock, (sockaddr*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
		err_quit("bind()");

	if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR)
		err_quit("listen()");

	std::cout << "Listen socket opened\n";
}

void Server::SendAllClient()
{
	for (int i = 0; i < 4; ++i)
	{
		// sending playerinfo packet
		char packetType = SC_PlayerInfo;
		for (int j = 0; i < 4; ++i)
		{
			// need conversion operator packet => char* 
			if (clients.at(i)->sock != NULL)
			{
				Client* c = clients.at(i);
				XMFLOAT3 movement{ c->xPos - c->oldxPos,
									c->yPos - c->oldyPos,
									c->zPos - c->oldzPos };
				XMFLOAT3 rotation{ c->pitch - c->oldPitch,
									c->yaw - c->oldYaw,
									c->roll - c->oldRoll };
				send(clients.at(i)->sock, (char*)&packetType, sizeof(char), 0);
				send(clients.at(i)->sock, (char*)&i, sizeof(char), 0);
				send(clients.at(i)->sock, (char*)&movement, sizeof(char), 0);
				send(clients.at(i)->sock, (char*)&rotation, sizeof(char), 0);
			}
		}
	}
}

void Server::Update()
{
}

void Server::CheckCollision()
{
	for (int i = 0; i < 4; ++i)
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
					players[i]->SetActive(false);
					missile.SetActive(false);
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

DWORD WINAPI AcceptClient(LPVOID arg)
{
	UNREFERENCED_PARAMETER(arg);

	SOCKET clientSock;
	sockaddr_in clientaddr;
	int addrlen;
	while (true)
	{
		addrlen = sizeof(clientaddr);
		// maybe a event need that notify server accepted four player already or not
		clientSock = accept(*g_server->GetSocket(), (sockaddr*)&clientaddr, &addrlen);
		std::cout << "Client accepted\n";
		if (clientSock == INVALID_SOCKET)
		{
			std::cout << "Invalid Socket detected\n";
			continue;
		}

		for (int i = 0; i < 4; ++i)
		{
			if (g_server->clients.at(i) == nullptr) {
				Client* client = new Client();
				client->sock = clientSock;
				client->SetPlayerNumber(i);
				g_server->clients.at(i) = client;
				break;
			}
		}
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[Server] Client Accepted: IP= %s, Port Number= %d\n",
			addr, ntohs(clientaddr.sin_port));
	}
	return 0;
}

DWORD WINAPI ReceiveAllClient(LPVOID arg)
{
	UNREFERENCED_PARAMETER(arg);

	sockaddr_in clientaddr{};
	char buf[BUFSIZE];

	while (true) {
		// Event off
		for (int i = 0; i < 1; ++i)
		{
			if (g_server->clients.at(i) != nullptr)
			{
				if (recv(g_server->clients.at(i)->sock, buf, BUFSIZE, 0) == SOCKET_ERROR)
					std::cout << "recv failed from " << i << " client\n";
				g_server->playerKey[i] = buf[0];
				g_server->playerKey[(int)buf[0]] = buf[1];
			}
		}
	}

	return 0;
}

int main()
{
	//Create Object Mgr
	g_server = new Server();
	g_server->OpenListenSocket();
	HANDLE AcceptThread, ReceiveThread;
	AcceptThread = CreateThread(NULL, 0, AcceptClient, nullptr, 0, NULL);
	ReceiveThread = CreateThread(NULL, 0, ReceiveAllClient, nullptr, 0, NULL);
	while (true)
	{
		// Event is on?
		g_server->Update();
		g_server->CheckCollision();
		// SendAllClient
	}
}





