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
	for (int i = 0; i < 10; i++)
	{
		m_ItemObject[i] = new CItemObject();
		m_ItemObject[i]->SetPosition(100, 100, 100 + 10 * i);
	}

	updateDone = CreateEvent(nullptr, true, false, nullptr);
}

Server::~Server()
{
	WSACleanup();
	for (int i = 0; i < 4; ++i)
	{
		if (clients[i] != nullptr)
			delete clients[i];
	}
	for (int i = 0; i < 10; i++)
	{
		if (m_ItemObject[i] != nullptr)
		{
			delete m_ItemObject[i];
		}
	}
}

Client::Client()
{
	m_player = new CPlayer();
}

Client::~Client()
{
	delete m_player;
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

void Server::CheckCollision()
{
	for (int i = 0; i < 4; ++i)
	{
		if (!clients[i]->IsConnected())
			continue;


		CPlayer* iPlayer = clients[i]->m_player;
		if (!iPlayer->IsActive())
			continue;

		//Collision Bottom
		//if (iPlayer->GetCurPos().y < 250.f)
		//	iPlayer->SetPosition(iPlayer->m_fOldxPos, iPlayer->m_fOldyPos, iPlayer->m_fOldzPos);


		for (int j = 0; j < 4; ++j)
		{
			//Same Player
			if (i == j)
				continue;
			if (!clients[j]->IsConnected())
				continue;


			CPlayer* jPlayer = clients[j]->m_player;
			if (!jPlayer->IsActive())
				continue;

			// Check Player to Player
			if (iPlayer->GetBoundingBox().Intersects(jPlayer->GetBoundingBox()))
			{
				iPlayer->m_nHp -= 10;
				iPlayer->SetPosition(iPlayer->m_fOldxPos, iPlayer->m_fOldyPos, iPlayer->m_fOldzPos);

				jPlayer->m_nHp -= 10;
				jPlayer->SetPosition(jPlayer->m_fOldxPos, jPlayer->m_fOldyPos, jPlayer->m_fOldzPos);
			}

			for (auto& missile : jPlayer->m_pMissiles)
			{
				if (!missile->IsActive())
					continue;

				// Check Players and Missiles
				if (iPlayer->GetBoundingBox().Intersects(missile->GetBoundingBox()))
				{
					iPlayer->m_nHp -= missile->damage;
					missile->ShouldDeactive();
					if (iPlayer->m_nHp <= 0)
					{
						iPlayer->Reset();
						iPlayer->ShouldDeactive();
					}
				}
			}
		}

		for (int j = 0; j < 10; j++)
		{
			if (!m_ItemObject[j]->IsActive())
				continue;

			if (iPlayer->GetBoundingBox().Intersects(m_ItemObject[j]->GetBoundingBox()))
			{
				iPlayer->m_nHp += m_ItemObject[j]->healAmount;
				if (iPlayer->m_nHp > 100)
					iPlayer->m_nHp = 100;

				m_ItemObject[j]->ShouldDeactive();
			}
		}
	}
}

void Server::SendAllClient()
{
	for (int i = 0; i < 4; ++i)		// client number
	{
		PlayerInfoPacket scInfo;

		if (clients[i]->IsConnected())
		{
			Client* c = clients[i];
			CPlayer* p = c->m_player;
			int playerNumber = c->GetPlayerNumber();

			XMFLOAT3 position{ p->m_fxPos, p->m_fyPos, p->m_fzPos };

			// PlayerInfo
			scInfo.packetType = SC_PlayerInfo;
			scInfo.playerNumber = playerNumber;
			scInfo.playerHP = p->m_nHp;
			scInfo.movement = position;
			scInfo.rotation = XMFLOAT3(p->m_fPitch, p->m_fYaw, p->m_fRoll);

			if ((scInfo.playerActive = !c->ShouldDisconnected()) == false)
			{
				c->Disconnect();			// disconnect
			}

			for (const auto& client : clients)
			{
				if (client->IsConnected())
				{
					send(client->sock, (char*)&scInfo, sizeof(PlayerInfoPacket), 0);

					for (int i = 0; i < 8; ++i)
					{
						CMissileObject* missile = p->m_pMissiles[i];
						if (missile->IsActive())
						{
							MissileInfoPacket scMissile;
							scMissile.packetType = SC_MissileInfo;
							scMissile.playerNumber = playerNumber;
							scMissile.missileNumber = i;
							if (missile->shouldDeactivated)
							{
								trashCan.push(missile);
								scMissile.active = false;
							}
							else
							{
								scMissile.active = true;
							}

							scMissile.movement = XMFLOAT3(missile->m_fxPos, missile->m_fyPos, missile->m_fzPos);
							scMissile.rotation = XMFLOAT3(missile->m_fPitch, missile->m_fYaw, missile->m_fRoll);
							send(client->sock, (char*)&scMissile, sizeof(MissileInfoPacket), 0);
						}
					}

					//sending ItemInfo Packet
					for (int i = 0; i < 10; ++i)
					{
						CItemObject* item = m_ItemObject[i];
						if (item->IsActive())
						{
							ItemInfoPacket scItem;
							scItem.packetType = SC_ItemInfo;
							if (item->shouldDeactivated)
							{
								trashCan.push(item);
								scItem.active = false;
							}
							else
							{
								scItem.active = true;
							}
							scItem.itemNum = i;
							scItem.position = item->GetCurPos();
							send(client->sock, (char*)&scItem, sizeof(ItemInfoPacket), 0);
						}
					}
				}
			}
		}
	}
}

void Server::Update()
{
	ResetEvent(updateDone);

	for (int i = 0; i < 4; ++i)
	{
		// connected, but dead
		if (clients[i]->IsConnected() && !clients[i]->m_player->IsActive())
		{
			clients[i]->deadTime += elapsedTime;
			if (clients[i]->deadTime > 5.f)
			{
				clients[i]->m_player->SetActive(true);
				clients[i]->deadTime = 0.f;
			}
		}
		else
		{
			clients[i]->m_player->Update(elapsedTime, g_server->connectedClients);
		}
	}

	SetEvent(updateDone);


	CheckCollision();
	SendAllClient();


	itemSpawnTime += elapsedTime;
	if (itemSpawnTime > 8.f)
	{
		itemSpawnTime -= 8.f;
		if (connectedClients >= 2)
			SpawnItem();
	}


	while (!trashCan.empty())
	{
		trashCan.front()->Deactivate();
		trashCan.pop();
	}
}

void Server::SpawnItem()
{
	for (int i = 0; i < 10; ++i)
	{
		if (!m_ItemObject[i]->IsActive())
		{
			m_ItemObject[i]->SetActive(true);
			m_ItemObject[i]->healAmount = ((rand() % 3) + 1) * 10;
			m_ItemObject[i]->SetPosition(rand() % 500, rand() % 500, rand() % 500);
			break;
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
		if (g_server->connectedClients < 4)
			std::cout << "Waiting for accept...\n";

		clientSock = accept(*g_server->GetSocket(), (sockaddr*)&clientaddr, &addrlen);
		if (clientSock == SOCKET_ERROR)
		{
			err_quit("accept()");
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
				HANDLE receiver = CreateThread(NULL, 0, ReceiveFromClient, client, 0, NULL);

				break;
			}
			continue;
		}
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("[Server] Client Accepted: IP=%s, Port Number=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}
	return 0;
}

DWORD WINAPI ReceiveFromClient(LPVOID arg)
{
	Client* client = (Client*)arg;

	int playerNumber = client->GetPlayerNumber();
	send(client->sock, (char*)&playerNumber, sizeof(int), 0);

	client->m_player->SetActive(true);
	client->m_player->SetPosition(g_server->initialPos[playerNumber].x, g_server->initialPos[playerNumber].y, g_server->initialPos[playerNumber].z);

	client->ToggleConnected();
	++g_server->connectedClients;

	PlayerKeyPacket keyPacket;

	while (true)
	{
		WaitForSingleObject(g_server->updateDone, INFINITE);
		if (recv(client->sock, (char*)&keyPacket, sizeof(PlayerKeyPacket), 0) == SOCKET_ERROR)
		{
			// cut the connection
			client->Reset();

			g_server->connectedClients--;

			break;
		}
		CPlayer* player = client->m_player;
		player->playerKey = keyPacket.playerKeyInput;

		player->m_deltaX = keyPacket.deltaMouse.x;
		player->m_deltaY = keyPacket.deltaMouse.y;
	}

	return 0;
}

void Client::Reset()
{
	closesocket(sock);
	sock = NULL;

	shouldDisconnected = true;


	//m_playerNumber = -1;
	m_player->Reset();
}

int main()
{
	g_server = new Server();
	g_server->OpenListenSocket();



	srand(time(NULL));


	HANDLE acceptThread = CreateThread(NULL, 0, AcceptClient, nullptr, 0, NULL);
	while (true)
	{
		g_server->elapsedTime = g_server->timer.GetTimePassedFromLastUpdate();
		g_server->timer.Record();
		g_server->Update();
	}
}

