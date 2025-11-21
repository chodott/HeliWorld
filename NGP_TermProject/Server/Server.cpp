#include "Server.h"
#include "GameObject.h"
#include "SCPacket.h"

Server* g_server;
std::unordered_map<int, vector<PlayerKeyPacket>> g_playerInputMap;
int g_serverTick = 0;


int PacketSizeHelper(char packetType)
{
	int packetSize;
	switch(packetType)
	{
	case CS_KeyInfo:
		packetSize = sizeof(PlayerKeyPacket);
		break;
	case CS_PingpongInfo:
		packetSize = sizeof(PingpongPacket);
		break;	
	default:
		packetSize = -1;
		break;
	}
	return packetSize;
}

Server::Server()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	for (int i = 0; i < MAX_CLIENT_NUM; ++i)
	{
		clients[i] = new Client;
		CPlayer* player = clients[i]->m_player;
		player->SetPosition(player->initialPos[i]);
		player->RotatePYR(player->initialRot[i]);
	}
	for (int i = 0; i < MAX_ITEM_NUM; i++)
	{
		m_ItemObject[i] = new CItemObject();
		m_ItemObject[i]->SetPosition(100.f, 100.f, 100.f + 10.f * i);
	}

	updateDone = CreateEvent(nullptr, true, false, nullptr);
	UpdateSnapshot(0);
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
	uint64_t resimulateStartTick = ReturnResimulateStart();
	uint64_t resetTick = resimulateStartTick;
	if (resimulateStartTick > 0)
	{
		resetTick--;
	}
	ResetToSnapshot(resetTick);
	for (uint64_t tick = resimulateStartTick; tick <= g_serverTick; ++tick)
	{
		for (int i = 0; i < MAX_CLIENT_NUM; ++i)
		{
			CPlayer* player = clients[i]->m_player;
			if (InputLogMaps[i].find(tick) != InputLogMaps[i].end())
			{
				player->keyPacket = InputLogMaps[i][tick];
			}
			player->Update(kDt);
		}
		CheckCollision();

		while (!trashCan.empty())
		{
			trashCan.front()->Deactivate();
			trashCan.pop();
		}
		UpdateSnapshot(tick);
		GenerateEvents(tick);
	}



	//for (int i = 0; i < MAX_CLIENT_NUM; ++i)
	//{
	//	CPlayer* player = clients[i]->m_player;
	//	if (clients[i]->keyPacket_q.try_pop(player->keyPacket));
	//	// connected, but dead
	//	if (clients[i]->IsConnected() && !player->IsActive())
	//	{
	//		clients[i]->deadTime += elapsedTime;
	//		if (clients[i]->deadTime > RESPAWN_TIME)
	//		{
	//			player->SetActive(true);
	//			clients[i]->deadTime = 0.f;
	//		}
	//	}
	//}

	itemSpawnTime += elapsedTime;
	if (itemSpawnTime > itemRespawnTime)
	{
		itemSpawnTime -= itemRespawnTime;
		if (connectedClients >= 2)
			SpawnItem();
	}

	PreparePackets();
	packetReadyCV.notify_one();
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

	for (int clientNum =0; clientNum < MAX_CLIENT_NUM;++clientNum)
	{
		Client* client = clients[clientNum];
		CPlayer* player = client->m_player;
		playerBundle.playerInfos[clientNum] = { clientNum, player->m_nHp, player->GetCurPos(), player->GetCurRot(), player->IsActive()};

		for (int i =0; i < player->maxMissileNum; ++i)
		{
			CMissileObject* missile = player->m_pMissiles[i];
			MissileInfoPacket& missileInfo = missileBundle.missileInfos[clientNum * player->maxMissileNum + i];
			missileInfo.position = missile->GetCurPos();
			missileInfo.active = missile->IsActive();
		}
	}

	playerBundle.serverTick = ++g_serverTick;

	for (int i=0;i<MAX_ITEM_NUM;++i)
	{
		CItemObject* item = m_ItemObject[i];
		ItemInfoPacket& itemInfo = itemBundle.itemInfos[i];
		ConvertFloat3toInt32(item->GetCurPos(), itemInfo.positionX, itemInfo.positionY, itemInfo.positionZ, MAP_SCALE);
		itemInfo.active = item->IsActive();
	}

	std::lock_guard<std::mutex> lock(packetQueueLock);
	PushPacket(playerBundle);
	PushPacket(missileBundle);
	PushPacket(itemBundle);
}

void Server::GenerateEvents(uint64_t tick)
{

	for (int playerNum = 0; playerNum < MAX_CLIENT_NUM; ++playerNum)
	{
		Client* client = clients[playerNum];
		CPlayer* player = client->m_player;
		ServerSnapshot& snapshot = SnapshotLogMap[tick - 1];
		for (int i = 0; i < MAX_MISSILE_NUM; ++i)
		{
			CMissileObject* missile = player->m_pMissiles[i];
			bool prevMissileActive = snapshot.missileSnapshots[playerNum * MAX_MISSILE_NUM + i].active;
			bool curMissileActive = missile->IsActive();
			if (prevMissileActive == curMissileActive)
			{
				continue;
			}

			if (curMissileActive == true && client->ShouldSendEvent(missile->GetID()) == false)
			{
				continue;
			}

			GetQueue<LocalMissileEventPacket>().push(
				{
					CS_LocalMissileEvent,
					missile->GetID(),
					tick,
					playerNum,
					missile->IsActive()
				}
			);
		}
	}
}

uint64_t Server::GetTimestampMs()
{
	using namespace std::chrono;
	return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>
		(steady_clock::now().time_since_epoch()).count();
}

void Server::PushInputData(int index, const PlayerKeyPacket& keyPacket)
{
	InputBuffers[index].push(keyPacket);
}

void Server::ResetToSnapshot(uint64_t targetTick)
{
	if (SnapshotLogMap.find(targetTick) == SnapshotLogMap.end())
	{
		return;
	}

	ServerSnapshot& snapshot = SnapshotLogMap[targetTick];
	for (int i = 0; i < MAX_CLIENT_NUM; ++i)
	{
		CPlayer* player = clients[i]->m_player;
		player->SetPosition(snapshot.playerSnapshots[i].position);
		player->RotatePYR(snapshot.playerSnapshots[i].rotation);
		player->SetHp(snapshot.playerSnapshots[i].hp);

		int startMissileIndex = i * MAX_MISSILE_NUM;
		for (int j = 0; j < 8; ++j)
		{
			CMissileObject* missile = player->m_pMissiles[j];
			missile->SetPosition(snapshot.missileSnapshots[startMissileIndex +j].position);
			missile->SetLifeTime(snapshot.missileSnapshots[startMissileIndex + j].lifeTime);
			missile->SetActive(snapshot.missileSnapshots[startMissileIndex + j].active);
		}
	}
	
	for (int i = 0; i < MAX_ITEM_NUM; ++i)
	{
		m_ItemObject[i]->SetPosition(snapshot.itemSnapshots[i].position);
	}

}

void Server::UpdateSnapshot(uint64_t targetTick)
{
	if (SnapshotLogMap.find(targetTick) == SnapshotLogMap.end())
	{
		ServerSnapshot empty;
		SnapshotLogMap.insert(make_pair(targetTick, empty));
	}

	ServerSnapshot& snapshot = SnapshotLogMap[targetTick];
	for (int i = 0; i < MAX_CLIENT_NUM; ++i)
	{
		CPlayer* player = clients[i]->m_player;
		snapshot.playerSnapshots[i].position =  player->GetCurPos();
		snapshot.playerSnapshots[i].rotation = player->GetCurRot();
		snapshot.playerSnapshots[i].hp = player->GetHp();

		int startMissileIndex = i * MAX_MISSILE_NUM;
		for (int j = 0; j < MAX_MISSILE_NUM; ++j)
		{
			CMissileObject* missile = player->m_pMissiles[j];
			snapshot.missileSnapshots[startMissileIndex + j].position = missile->GetCurPos();
			snapshot.missileSnapshots[startMissileIndex + j].lifeTime = missile->GetLifeTime();
			snapshot.missileSnapshots[startMissileIndex + j].active = missile->IsActive();
		}
	}

	for (int i = 0; i < MAX_ITEM_NUM; ++i)
	{
		snapshot.itemSnapshots[i].position = m_ItemObject[i]->GetCurPos();
	}
}

uint64_t Server::ReturnResimulateStart()
{
	int startTick = g_serverTick;
	for (int index = 0; index < MAX_CLIENT_NUM; ++index)
	{
		while (!InputBuffers[index].empty())
		{
			uint64_t tick = InputBuffers[index].front().estimatedTick;
			if (tick > g_serverTick)
			{
				break;
			}

			InputLogMaps[index][tick] = InputBuffers[index].front();
			InputBuffers[index].pop();
			startTick = min(startTick, tick);
		}
	}
	return startTick;
}


void Server::SendPacketAllClient()
{
	PlayerInfoBundlePacket playerInfoBundle;
	MissileInfoBundlePacket missileInfoBundle;
	ItemInfoBundlePacket itemInfoBundle;

	{
		std::unique_lock<std::mutex> lock(packetQueueLock);
		packetReadyCV.wait(lock, [this] {
			return !GetQueue<PlayerInfoBundlePacket>().empty() &&
				!GetQueue<MissileInfoBundlePacket>().empty() &&
				!GetQueue<ItemInfoBundlePacket>().empty();
			});
	}

	TryPopPacket(playerInfoBundle);
	TryPopPacket(missileInfoBundle);
	TryPopPacket(itemInfoBundle);

	for (const auto& client : clients)
	{
		if (!client->IsConnected())
		{
			continue;
		}

		SOCKET& recvSock = client->sock;
		SendPacket(recvSock, playerInfoBundle);
		SendPacket(recvSock, missileInfoBundle);
		SendPacket(recvSock, itemInfoBundle);
	}
	LocalMissileEventPacket missileEventPacket;
	while (TryPopPacket(missileEventPacket) == true)
	{
		SOCKET& recvSock = clients[missileEventPacket.playerNum]->sock;
		SendPacket(recvSock, missileEventPacket);
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
		bool noDelay = true;
		setsockopt(clientSock, IPPROTO_TCP, TCP_NODELAY, (char*)noDelay, sizeof(noDelay));

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

	//Init Packet
	Client* client = (Client*)arg;
	int playerNumber = client->GetPlayerNumber();
	TimebasePacket timebasePacket{ playerNumber, g_serverTick, g_server->GetTimestampMs() };
	send(client->sock, (char*)&timebasePacket, sizeof(timebasePacket), 0);

	CPlayer* p = client->m_player;
	p->SetActive(true);
	XMFLOAT3& initialPosition = p->initialPos[playerNumber];
	XMFLOAT3& initialRotation = p->initialRot[playerNumber];
	p->SetPosition(initialPosition.x, initialPosition.y, initialPosition.z);
	p->Rotate(initialRotation.x, initialRotation.y, initialRotation.z);

	client->Connected();
	++g_server->connectedClients;
	PlayerKeyPacket& keyPacket = g_server->keyPackets[playerNumber];

	const int bufMaxSize = 512;
	int combinedSize = 0;
	int remainOffset = 0;
	int receivedBytes;
	char buf[512]{};
	while (true)
	{
		receivedBytes = recv(client->sock, (char*)&buf, bufMaxSize, 0);
		if(receivedBytes == SOCKET_ERROR)
		{
			// cut the connection
			client->Reset();
			--g_server->connectedClients;
			break;
		}
		
		int bufSize = bufMaxSize;
		int offset = 0;

		memcpy(client->remainBuffer + remainOffset, buf, receivedBytes);
		combinedSize += receivedBytes;

		while (offset < combinedSize)
		{
			char packetType = buf[offset];
			int packetSize = PacketSizeHelper(packetType);
			   
			if (offset + packetSize > combinedSize)
			{
				//next packet
				break;
			}

			switch (packetType)
			{
			case CS_KeyInfo:
			{
				CPlayer* player = client->m_player;
				memcpy(&keyPacket, client->remainBuffer + offset, packetSize);
				client->keyPacket_q.push(keyPacket);
				g_server->PushInputData(client->GetPlayerNumber(), keyPacket);
				break;
			}
			case CS_PingpongInfo:
			{
				PingpongPacket ppPacket;
				memcpy(&ppPacket, client->remainBuffer + offset, packetSize);
				ppPacket.serverSendTimeStamp = g_server->GetTimestampMs();
				send(client->sock, (char*)&ppPacket, PacketSizeHelper(CS_PingpongInfo), 0);
				break;
			}
			}
			offset += packetSize;

		}
		int remainSize = combinedSize - offset;
		memmove(client->remainBuffer, client->remainBuffer + offset, remainSize); // �����ϰ� �ű��
		combinedSize = remainSize;
	}

	return 0;
}

DWORD WINAPI SendAllClient(LPVOID arg)
{
	while (1)
	{
		g_server->SendPacketAllClient();
	}
}

bool Client::ShouldSendEvent(uint64_t id)
{
	if (lastLaunchedMissileID >= id)
	{
		return false;
	}
	lastLaunchedMissileID = id;
	return true;
}

void Client::Reset()
{
	closesocket(sock);
	sock = NULL;

	lastLaunchedMissileID = 0;
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

	auto  prev = std::chrono::steady_clock::now();
	double acc = 0.0;

	while (true)
	{
		//Tick Base
		auto now = std::chrono::steady_clock::now();
		double frameDelta = std::chrono::duration<double>(now - prev).count();
		prev = now;
		acc += frameDelta;

		int steps = 0, maxSteps = 6;
		while (acc >= kDt && steps < maxSteps)
		{
			g_server->Update();
			acc -= kDt; ++steps;
		}
	}

	
}

