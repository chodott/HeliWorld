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

			XMFLOAT3 position{ p->m_fxPos, p->m_fyPos, p->m_fzPos };
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
			Client* client = g_server->clients[i];
			if (!client->IsConnected())
			{
				client->sock = clientSock;
				client->SetPlayerNumber(i);

				std::cout << "Client accepted in " << client->GetPlayerNumber() << std::endl;
				HANDLE receiver = CreateThread(NULL, 0, ReceiveAllClient, client, 0, NULL);

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
	Client* client = (Client*)arg;

	int playerNumber = client->GetPlayerNumber();
	send(client->sock, (char*)&playerNumber, sizeof(int), 0);

	client->ToggleConnected();
	client->m_player->SetActive(true);
	client->m_player->SetPosition(g_server->initialPos[playerNumber].x, g_server->initialPos[playerNumber].y, g_server->initialPos[playerNumber].z);


	char buf[1]{};

	while (true)
	{
		if (client->IsConnected())
		{
			if (recv(client->sock, buf, 1, 0) == SOCKET_ERROR)
			{
				// cut the connection
				continue;
			}
			client->m_player->playerKey = buf[0];
		}
	}

	return 0;
}

int main()
{
	//Create Object Mgr
	g_server = new Server();
	g_server->OpenListenSocket();
	HANDLE AcceptThread = CreateThread(NULL, 0, AcceptClient, nullptr, 0, NULL);
	while (true)
	{
		g_server->Update();
	}
}





