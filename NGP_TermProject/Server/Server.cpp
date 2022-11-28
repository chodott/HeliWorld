#include "Server.h"
#include "GameObject.h"

Server* g_server;

Server::Server()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	for (int i = 0; i < 4; ++i)
	{
		//ego jonna byeollo - Chodot
		clients[i] = new Client;
		//players[i] = new CPlayer;
	}
}

Client::Client()
{
	m_player = new CPlayer();
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
				GameObject* p = clients.at(i)->m_player;
				XMFLOAT3 movement{ p->m_fxPos - p->m_fOldxPos,
							   p->m_fyPos - p->m_fOldyPos,
							   p->m_fzPos - p->m_fOldzPos };
				XMFLOAT3 rotation{ p->m_fPitch - p->m_fOldPitch,
							   p->m_fYaw - p->m_fOldYaw,
							   p->m_fRoll - p->m_fOldRoll };
				PlayerInfoPacket playerInfo;
				playerInfo.packetType = packetType;
				playerInfo.playerNumber = i;
				playerInfo.movement = movement;
				playerInfo.rotation = rotation;

				send(clients.at(i)->sock, (char*)&playerInfo, sizeof(PlayerInfoPacket), 0);
			}
		}
	}
}

void Server::Update()
{
	WaitForSingleObject(g_server->ReceiveEvent, INFINITE);
	int n = 0;
	for (int i = 0; i < 4; ++i)
	{
		//players[i]->Move(playerKey[i], 1.f, true);
		//clients[i]->m_player->Move(playerKey[i], 1.f, true);

		clients[i]->m_player->Move(clients[i]->m_player->playerKey, 0.5f, true);

	}

	CheckCollision();
	SendAllClient();
}

void Server::CheckCollision()
{
	for (int i = 0; i < 4; ++i)
	{
		//if (players[i] == nullptr) continue;
		if (!clients[i]->IsConnected()) continue;

		CPlayer* iPlayer = clients[i]->m_player;
		if (!iPlayer->GetActive()) continue;


		for (int j = 0; j < 4; ++j)
		{
			if (i == j) continue;		//Same Player
			if (!clients[j]->IsConnected()) continue;
			//else if (players[j] == nullptr) continue;

			CPlayer* jPlayer = clients[j]->m_player;

			//Collision Players
			//if (players[j]->GetActive() && players[i]->GetBoundingBox().Intersects(players[j]->GetBoundingBox()))
			if (!jPlayer->GetActive()) continue;
			if (iPlayer->GetBoundingBox().Intersects(jPlayer->GetBoundingBox()))
			{
				// function will be called
			}

			for (auto& missile : jPlayer->m_pMissiles)
			{
				//Collision between Player and Missiles
				if (iPlayer->GetBoundingBox().Intersects(missile.GetBoundingBox()))
				{
					iPlayer->SetActive(false);
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
			//if (g_server->clients.at(i) == nullptr)
			if (g_server->clients[i]->IsConnected() == false)
			{
				Client* client = new Client();
				client->sock = clientSock;
				client->SetPlayerNumber(i);
				g_server->clients[i] = client;


				CPlayer* player = g_server->clients[i]->m_player;
				//Set Player Initial Pos
				player->SetPosition(g_server->initialPos[i][0], g_server->initialPos[i][1], g_server->initialPos[i][2]);

				send(g_server->clients[i]->sock, (char*)&i, sizeof(int), 0);

				//Set Player Initial Rotation
				switch (i)
				{
				case 0:
					player->Rotate(0.f, 0.f, 0.f);
					break;
				case 1:
					player->Rotate(0.f, -45.f, 0.f);
					break;
				case 2:
					player->Rotate(0.f, 90.f, 0.f);
					break;
				case 3:
					player->Rotate(0.f, 45.f, 0.f);
					break;
				}

				g_server->clients[i]->ToggleConnected();
				break;
			}
		}
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("[Server] Client Accepted: IP= %s, Port Number= %d\n",
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
		ResetEvent(g_server->ReceiveEvent);
		for (int i = 0; i < 4; ++i)
		{
			//if (g_server->clients.at(i) != nullptr)
			if (g_server->clients[i]->IsConnected() != false)
			{
				if (recv(g_server->clients[i]->sock, buf, BUFSIZE, 0) == SOCKET_ERROR)
					std::cout << "recv failed from " << i << " client\n";
				//g_server->playerKey[i] = buf[0];
				g_server->clients[i]->m_player->playerKey = buf[0];
				//cout << g_server->playerKey[i] << endl;


				//g_server->playerKey[(int)buf[0]] = buf[1];
			}
		}
		SetEvent(g_server->ReceiveEvent);
		// Is the event terminated immediately after run?
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
	g_server->ReceiveEvent = CreateEvent(NULL, true, false, NULL);
	while (true)
	{
		// Event is on?
		g_server->Update();
		// SendAllClient
	}
}





