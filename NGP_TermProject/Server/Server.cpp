#include "Server.h"
#include "GameObject.h"

Server* g_server;

Server::Server()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	for (int i = 0; i < 4; ++i)
	{
		clients[i] = new Client;
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


	u_long nonBlockingMode = 1;
	ioctlsocket(listenSock, FIONBIO, &nonBlockingMode);


	std::cout << "Listen socket opened\n";
}

void Server::SendAllClient()
{
	for (int i = 0; i < 4; ++i)		// client number
	{
		char packetType = SC_PlayerInfo;
		PlayerInfoPacket toSend;
		toSend.packetType = packetType;
		if (clients[i]->IsConnected())
		{
			Client* c = clients.at(i);
			GameObject* p = c->m_player;

			XMFLOAT3 position{ p->m_fxPos, p->m_fyPos, p->m_fzPos};
			XMFLOAT3 rotation{ p->m_fPitch - p->m_fOldPitch,
						   p->m_fYaw - p->m_fOldYaw,
						   p->m_fRoll - p->m_fOldRoll };	// to Update function, into infopackets

			toSend.playerNumber = c->GetPlayerNumber();
			toSend.movement = position;
			toSend.rotation = rotation;
		}

		// sending playerinfo packet
		for (const auto& client : clients)
		{
			if (client->IsConnected())
			{
				send(client->sock, (char*)&toSend, sizeof(PlayerInfoPacket), 0);
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
		clients[i]->m_player->Move(clients[i]->m_player->playerKey, 0.0005f, true);
	}

	CheckCollision();
	SendAllClient();
}

void Server::CheckCollision()
{
	for (int i = 0; i < 4; ++i)
	{
		if (!clients[i]->IsConnected()) continue;

		CPlayer* iPlayer = clients[i]->m_player;
		if (!iPlayer->GetActive()) continue;


		for (int j = 0; j < 4; ++j)
		{
			if (i == j) continue;		//Same Player
			if (!clients[j]->IsConnected()) continue;

			CPlayer* jPlayer = clients[j]->m_player;

			//Collision Players
			if (!jPlayer->GetActive()) continue;
			if (iPlayer->GetBoundingBox().Intersects(jPlayer->GetBoundingBox()))
			{
				// function will be called
				iPlayer->SetPosition(iPlayer->m_fOldxPos, iPlayer->m_fOldyPos, iPlayer->m_fOldzPos);
				jPlayer->SetPosition(jPlayer->m_fOldxPos, jPlayer->m_fOldyPos, jPlayer->m_fOldzPos);
				cout << "충돌스키" << endl;
			}

			for (auto& missile : jPlayer->m_pMissiles)
			{
				if (!missile.GetActive()) continue;
				//Collision between Player and Missiles
				if (iPlayer->GetBoundingBox().Intersects(missile.GetBoundingBox()))
				{
					iPlayer->m_nHp -= missile.damage;
					missile.SetActive(false);
					if (iPlayer->m_nHp <= 0)
					{
						iPlayer->SetActive(false);
					}

				}
			}

		}
	}
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
		if (clientSock == INVALID_SOCKET)
		{
			//std::cout << "Invalid Socket detected\n";
			continue;
		}

		for (int i = 0; i < 4; ++i)
		{
			if (g_server->clients[i]->IsConnected() == false)
			{
				g_server->clients[i]->sock = clientSock;
				g_server->clients[i]->SetPlayerNumber(i);

				std::cout << "Client accepted in " <<
					g_server->clients[i]->GetPlayerNumber() << std::endl;


				send(g_server->clients[i]->sock, (char*)&i, sizeof(int), 0);
				g_server->clients[i]->ToggleConnected();


				CPlayer* player = g_server->clients[i]->m_player;
				player->SetActive(true);

				break;
			}
		}
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("[Server] Client Accepted: IP=%s, Port Number=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}
	return 0;
}

DWORD WINAPI ReceiveAllClient(LPVOID arg)
{
	UNREFERENCED_PARAMETER(arg);

	sockaddr_in clientaddr{};

	while (true) {
		// Event off
		ResetEvent(g_server->ReceiveEvent);
		for (int i = 0; i < 4; ++i)
		{
			if (g_server->clients[i]->IsConnected() != false)
			{
				if (recv(g_server->clients[i]->sock, g_server->clients[i]->buf, BUFSIZE, 0) == SOCKET_ERROR)
					g_server->clients[i]->m_player->playerKey = g_server->clients[i]->buf[0];
			}
		}
		SetEvent(g_server->ReceiveEvent);
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
		g_server->Update();
	}
}





