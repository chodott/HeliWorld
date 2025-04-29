#include "Server.h"
#include "GameObject.h"

Server* g_server;

Server::Server()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	for (int i = 0; i < MAX_CLIENT_NUM; ++i)
	{
		clients[i] = new Client;
	}
	for (int i = 0; i < MAX_ITEM_NUM; i++)
	{
		m_ItemObject[i] = new CItemObject();
		m_ItemObject[i]->SetPosition(100.f, 100.f, 100.f + 10.f * i);
	}

	updateDone = CreateEvent(nullptr, true, false, nullptr);

}

Server::~Server()
{
	WSACleanup();
	for (int i = 0; i < MAX_CLIENT_NUM; ++i)
	{
		if (clients[i] != nullptr)
			delete clients[i];
	}
	for (int i = 0; i < MAX_ITEM_NUM; i++)
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
	for (int i = 0; i < MAX_CLIENT_NUM; ++i)
	{
		if (!clients[i]->IsConnected())
			continue;

		CPlayer* iPlayer = clients[i]->m_player;
		if (!iPlayer->IsActive())
			continue;

		//Collision Map
		if (clients[i]->m_player->m_fyPos < MIN_BOUNDARY_Y || clients[i]->m_player->m_fyPos > MAX_BOUNDARY_Y ||
			clients[i]->m_player->m_fzPos < MIN_BOUNDARY_Z || clients[i]->m_player->m_fzPos > MAX_BOUNDARY_Z ||
			clients[i]->m_player->m_fxPos < MIN_BOUNDARY_X || clients[i]->m_player->m_fxPos > MAX_BOUNDARY_X)
		{
			clients[i]->m_player->SetPosition(clients[i]->m_player->m_fOldxPos,
				clients[i]->m_player->m_fOldyPos,
				clients[i]->m_player->m_fOldzPos);
		}

		for (int j = 0; j < MAX_CLIENT_NUM; ++j)
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
				//iPlayer->m_nHp -= 10;
				iPlayer->SetPosition(iPlayer->m_fOldxPos, iPlayer->m_fOldyPos, iPlayer->m_fOldzPos);

				//jPlayer->m_nHp -= 10;
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
					trashCan.push(missile);
					if (iPlayer->m_nHp <= 0)
					{
						iPlayer->Reset(i);
						iPlayer->ShouldDeactive();
					}
				}
			}
		}

		for (int j = 0; j < MAX_ITEM_NUM; j++)
		{
			if (!m_ItemObject[j]->IsActive())		continue;

			if (iPlayer->GetBoundingBox().Intersects(m_ItemObject[j]->GetBoundingBox()))
			{
				iPlayer->m_nHp += m_ItemObject[j]->healAmount;
				if (iPlayer->m_nHp > iPlayer->maxHp)			iPlayer->m_nHp = iPlayer->maxHp;

				m_ItemObject[j]->ShouldDeactive();
				trashCan.push(m_ItemObject[j]);
			}
		}
	}
}

void Server::Update()
{
	ResetEvent(updateDone);

	std::chrono::steady_clock::time_point curTime = std::chrono::steady_clock::now();
	for (int i = 0; i < MAX_CLIENT_NUM; ++i)
	{
		clients[i]->keyPacket_q.try_pop(clients[i]->m_player->keyPacket);
		// connected, but dead
		if (clients[i]->IsConnected() && !clients[i]->m_player->IsActive())
		{
			clients[i]->deadTime += elapsedTime;
			if (clients[i]->deadTime > RESPAWN_TIME)
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
	CheckCollision();

	itemSpawnTime += elapsedTime;
	if (itemSpawnTime > itemRespawnTime)
	{
		itemSpawnTime -= itemRespawnTime;
		if (connectedClients >= 2)
			SpawnItem();
	}

	while (!trashCan.empty())
	{
		trashCan.front()->Deactivate();
		trashCan.pop();
	}

	PreparePackets();
	SetEvent(updateDone);
}

void Server::SpawnItem()
{
	for (int i = 0; i < MAX_ITEM_NUM; ++i)
	{
		if (!m_ItemObject[i]->IsActive())
		{
			m_ItemObject[i]->SetActive(true);
			m_ItemObject[i]->healAmount = ((rand() % 3) + 1) * 10;
			m_ItemObject[i]->SetPosition(rand() % MAX_BOUNDARY_X, 
															rand() % (MAX_BOUNDARY_Y - MIN_BOUNDARY_Y) + MIN_BOUNDARY_Y, 
															rand() % MAX_BOUNDARY_Z);
			break;
		}
	}
}

void Server::PreparePackets()
{
	PlayerInfoBundlePacket playerBundle;
	MissileInfoBundlePacket missileBundle;
	ItemInfoBundlePacket itemBundle;
	playerBundle.packetType = SC_PlayerInfo;
	missileBundle.packetType = SC_MissileInfo;
	itemBundle.packetType = SC_ItemInfo;
	for (int clientNum =0; clientNum < MAX_CLIENT_NUM;++clientNum)
	{
		Client* client = clients[clientNum];
		CPlayer* player = client->m_player;
		playerBundle.playerInfos[clientNum] = { clientNum, player->m_nHp, player->GetCurPos(), player->GetCurRot(), player->IsActive()};
		for (int i =0; i < player->maxMissileNum; ++i)
		{
			CMissileObject* missile = player->m_pMissiles[i];
			MissileInfoPacket& missileInfo = missileBundle.missileInfos[clientNum * player->maxMissileNum + i];
			ConvertFloat3toInt16(missile->GetCurPos(), missileInfo.positionX, missileInfo.positionY, missileInfo.positionZ,MAP_SCALE);
			missileInfo.active = missile->IsActive();
		}
	}
	playerBundlePacket_q.push(playerBundle);
	missileBundlePacket_q.push(missileBundle);

	for (int i=0;i<MAX_ITEM_NUM;++i)
	{
		CItemObject* item = m_ItemObject[i];
		ItemInfoPacket& itemInfo = itemBundle.itemInfos[i];
		ConvertFloat3toInt16(item->GetCurPos(), itemInfo.positionX, itemInfo.positionY, itemInfo.positionZ, MAP_SCALE);
		itemInfo.active = item->IsActive();
	}
	itemBundlePacket_q.push(itemBundle);
}

uint32_t Server::GetTimestampMs()
{
	using namespace std::chrono;
	return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>
		(steady_clock::now().time_since_epoch()).count();
}


void Server::SendPacketAllClient(char* packet, int size, int flag)
{
	// send client[i] info to all clients
	for (const auto& client : clients)
	{
		if (!client->IsConnected())
		{
			continue;
		}
		send(client->sock, packet, size, flag);
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
		if (g_server->connectedClients < MAX_CLIENT_NUM)
			std::cout << "Waiting for accept...\n";

		clientSock = accept(*g_server->GetSocket(), (sockaddr*)&clientaddr, &addrlen);
		if (clientSock == SOCKET_ERROR)
		{
			err_quit("accept()");
			continue;
		}

		for (int i = 0; i < MAX_CLIENT_NUM; ++i)
		{
			Client* client = g_server->clients[i];
			client->SetPlayerNumber(i);

			if (!client->IsConnected())
			{
				client->sock = clientSock;

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

	CPlayer* p = client->m_player;
	p->SetActive(true);
	XMFLOAT3& initialPosition = p->initialPos[playerNumber];
	XMFLOAT3& initialRotation = p->initialRot[playerNumber];
	p->SetPosition(initialPosition.x, initialPosition.y, initialPosition.z);
	p->Rotate(initialRotation.x, initialRotation.y, initialRotation.z);

	client->Connected();
	++g_server->connectedClients;
	PlayerKeyPacket& keyPacket = g_server->keyPackets[playerNumber];
	while (true)
	{
		WaitForSingleObject(g_server->updateDone, INFINITE);
		if (recv(client->sock, (char*)&keyPacket, sizeof(PlayerKeyPacket), 0) == SOCKET_ERROR)
		{
			// cut the connection
			client->Reset();
			--g_server->connectedClients;
			break;
		}
		CPlayer* player = client->m_player;
		client->keyPacket_q.push(keyPacket);
	}

	return 0;
}

DWORD WINAPI SendAllClient(LPVOID arg)
{
	while (1)
	{
		PlayerInfoBundlePacket scInfoBundle;
		MissileInfoBundlePacket msInfoBundle;
		ItemInfoBundlePacket ItemInfoBundle;
		if (g_server->playerBundlePacket_q.try_pop(scInfoBundle))
		{
			g_server->SendPacketAllClient((char*)&scInfoBundle, sizeof(PlayerInfoBundlePacket), 0);
		}
		else
		{
			//Didnt prepare Packet Wait event
			WaitForSingleObject(g_server->updateDone, INFINITE);
			continue;
		}

		if (g_server->missileBundlePacket_q.try_pop(msInfoBundle))
		{
			g_server->SendPacketAllClient((char*)&msInfoBundle, sizeof(MissileInfoBundlePacket), 0);

		}

		//sending ItemInfo Packet
		if (g_server->itemBundlePacket_q.try_pop(ItemInfoBundle))
		{
			g_server->SendPacketAllClient((char*)&ItemInfoBundle, sizeof(ItemInfoBundlePacket), 0);

		}
		ResetEvent(g_server->updateDone);
	}
}

void Client::Reset()
{
	closesocket(sock);
	sock = NULL;

	shouldDisconnected = true;
	keyPacket_q.clear();
	Disconnect();
	m_player->Reset(GetPlayerNumber());
}

int main()
{
	g_server = new Server();
	g_server->OpenListenSocket();

	srand(time(NULL));
	HANDLE acceptThread = CreateThread(NULL, 0, AcceptClient, nullptr, 0, NULL);
	CreateThread(NULL, 0, SendAllClient, g_server, 0, NULL);

	while (true)
	{
		if (g_server->connectedClients < 1) continue;
		g_server->elapsedTime = g_server->timer.GetTimePassedFromLastUpdate();
		if (g_server->elapsedTime >= g_server->FIXED_DELTA_TIME)
		{
			g_server->timer.Record();
			g_server->Update();
			//g_server->SendAllClient();
			g_server->elapsedTime = g_server->FIXED_DELTA_TIME;
		}
	}
}

