#include "Server.h"
#include "GameObject.h"
#include "SCPacket.h"

Server* g_server;

int PacketSizeHelper(char packetType)
{
	int packetSize;
	switch (packetType)
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

	for (int i = 0; i < Protocol::kMaxPlayerCount; ++i)
	{
		clients[i] = new Client;
		CPlayer* player = clients[i]->m_player;
		player->SetPosition(player->initialPos[i]);
		player->RotatePYR(player->initialRot[i]);
	}


	updateDone = CreateEvent(nullptr, true, false, nullptr);
}

Server::~Server()
{
	WSACleanup();
	for (int i = 0; i < Protocol::kMaxPlayerCount; ++i)
	{
		if (clients[i] != nullptr)
			delete clients[i];
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




void Server::PreparePackets()
{
	TickSnapshotPacket tickSnapshot{};
	tickSnapshot.serverTick = ++serverTick;

	for (int clientNum = 0; clientNum < Protocol::kMaxPlayerCount; ++clientNum)
	{
		Client* client = clients[clientNum];
		CPlayer* player = client->m_player;
		tickSnapshot.playerInfos[clientNum] = { clientNum, player->m_nHp, player->GetCurPos(), player->GetCurRot(), player->IsActive() };

		for (int i = 0; i < Protocol::kMaxMissileCountPerPlayer; ++i)
		{
			CMissileObject* missile = player->m_pMissiles[i];
			MissileInfoPacket& missileInfo = tickSnapshot.missileInfos[clientNum * Protocol::kMaxMissileCountPerPlayer + i];
			missileInfo.position = missile->GetCurPos();
			missileInfo.active = missile->IsActive();
		}
	}

	for (int i = 0; i < Protocol::kMaxItemCount; ++i)
	{
		CItemObject* item = m_ItemObject[i];
		ItemInfoPacket& itemInfo = tickSnapshot.itemInfos[i];
		ConvertFloat3toInt32(item->GetCurPos(), itemInfo.positionX, itemInfo.positionY, itemInfo.positionZ, MAP_SCALE);
		itemInfo.active = item->IsActive();
	}

	PushPacket(tickSnapshot);
}

void Server::GenerateEvents(uint64_t tick)
{
	if (tick == 0) return;

	for (int playerNum = 0; playerNum < Protocol::kMaxPlayerCount; ++playerNum)
	{
		Client* client = clients[playerNum];
		CPlayer* player = client->m_player;
		ServerSnapshot& snapshot = SnapshotLogMap.at(tick - 1);
		for (int i = 0; i < Protocol::kMaxMissileCountPerPlayer; ++i)
		{
			CMissileObject* missile = player->m_pMissiles[i];
			bool prevMissileActive = snapshot.missileSnapshots[playerNum * Protocol::kMaxMissileCountPerPlayer + i].active;
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

void Server::SendPacketAllClient()
{
	TickSnapshotPacket tickSnapshotPacket;

	bool bCanSendPacket = TryPopPacket(tickSnapshotPacket);
	
	if (bCanSendPacket)
	{
		for (const auto& client : clients)
		{
			if (!client->IsConnected())
			{
				continue;
			}

			SOCKET& recvSock = client->sock;
			SendPacket(recvSock, tickSnapshotPacket);
		}
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
		if (g_server->connectedClients < Protocol::kMaxPlayerCount)
			std::cout << "Waiting for accept...\n";

		clientSock = accept(*g_server->GetSocket(), (sockaddr*)&clientaddr, &addrlen);
		if (clientSock == SOCKET_ERROR)
		{
			err_quit("accept()");
			continue;
		}
		BOOL noDelay = true;
		setsockopt(clientSock, IPPROTO_TCP, TCP_NODELAY, (char*)&noDelay, sizeof(noDelay));

		for (int i = 0; i < Protocol::kMaxPlayerCount; ++i)
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
	TimebasePacket timebasePacket{ playerNumber, g_server->GetTick(), g_server->GetTimestampMs() };
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

	int combinedSize = 0;
	char buf[BUFSIZE]{};
	while (true)
	{
		int receivedBytes = recv(client->sock, (char*)&buf, BUFSIZE, 0);
		if (receivedBytes == SOCKET_ERROR)
		{
			// cut the connection
			client->Reset();
			--g_server->connectedClients;
			break;
		}

		if (combinedSize + receivedBytes > BUFSIZE)
		{
			std::cout << "receive buffer overflow, dropping pending bytes\n";
			combinedSize = 0;
		}

		memcpy(client->remainBuffer + combinedSize, buf, receivedBytes);
		combinedSize += receivedBytes;

		int offset = 0;

		while (offset < combinedSize)
		{
			char packetType = client->remainBuffer[offset];
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

		if (combinedSize == 0)
		{
			continue;
		}

		int remainSize = combinedSize - offset;
		if (remainSize > 0)
		{
			memmove(client->remainBuffer, client->remainBuffer + offset, remainSize);
		}
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

	ClientInputBuffer* clientInputBuffer = new ClientInputBuffer();
	SimulationServer* simulationServer = new SimulationServer();

	srand(time(NULL));
	HANDLE acceptThread = CreateThread(NULL, 0, AcceptClient, nullptr, 0, NULL);
	HANDLE sendThread = CreateThread(NULL, 0, SendAllClient, g_server, 0, NULL);

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
		while (acc >= Protocol::kFixedTick && steps < maxSteps)
		{
			simulationServer->Update();
			acc -= Protocol::kFixedTick;
			++steps;
		}

		if (acc < Protocol::kFixedTick)
		{
			double margin = Protocol::kFixedTick - acc;
			DWORD sleepMs = (DWORD)(margin * 1000.0);
			Sleep(sleepMs);
		}
	}

	CloseHandle(acceptThread);
	CloseHandle(sendThread);
}

