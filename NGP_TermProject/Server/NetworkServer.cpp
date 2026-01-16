#include "NetworkServer.h"

DWORD WINAPI AcceptClient(LPVOID arg)
{
	ServerContext* serverContext = static_cast<ServerContext*>(arg);
	NetworkServer* netServer = serverContext->netServer;

	SOCKET clientSock;
	sockaddr_in clientaddr;

	int addrlen;
	while (netServer->IsRunning())
	{
		addrlen = sizeof(clientaddr);
		if (netServer->connectedClients < Protocol::kMaxPlayerCount)
			std::cout << "Waiting for accept...\n";

		clientSock = accept(*netServer->GetSocket(), (sockaddr*)&clientaddr, &addrlen);
		if (clientSock == SOCKET_ERROR)
		{
			err_quit("accept()");
			continue;
		}
		BOOL noDelay = true;
		setsockopt(clientSock, IPPROTO_TCP, TCP_NODELAY, (char*)&noDelay, sizeof(noDelay));

		for (int i = 0; i < Protocol::kMaxPlayerCount; ++i)
		{
			Client* client = netServer->clients[i];
			client->SetPlayerNumber(i);

			if (!client->IsConnected())
			{
				client->Connect(clientSock, i, serverContext);
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

DWORD WINAPI SendAllClient(LPVOID arg)
{
	ServerContext* serverContext = static_cast<ServerContext*>(arg);
	NetworkServer* netServer = serverContext->netServer;
	SnapshotPacketBuffer* snapshotBuffer = serverContext->snapshotBuffer;
	while (netServer->IsRunning())
	{
		WaitForSingleObject(snapshotBuffer->GetSendEvent(), INFINITE);
		netServer->SendPacketAllClient();
	}
}


NetworkServer::NetworkServer(ClientInputBuffer& inputBuffer, SnapshotPacketBuffer& packetBuffer, NetworkEventQueue& networkEventQueue):
	clientInputBuffer(inputBuffer), snapshotPacketBuffer(packetBuffer), eventQueue(networkEventQueue)
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	for (int i = 0; i < Protocol::kMaxPlayerCount; ++i)
	{
		clients[i] = new Client;
	}

}

NetworkServer::~NetworkServer()
{
	WSACleanup();
	for (int i = 0; i < Protocol::kMaxPlayerCount; ++i)
	{
		if (clients[i] != nullptr)
			delete clients[i];
	}
}

void NetworkServer::OpenListenSocket(ServerContext* serverContext)
{
	// create listen socket
	if ((listenSock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) err_quit("socket()");

	srand(time(NULL));
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

	srand(time(NULL));

	acceptHandle = CreateThread(NULL, 0, AcceptClient, serverContext, 0, NULL);
	sendHandle = CreateThread(NULL, 0, SendAllClient, serverContext, 0, NULL);
	isRunning = true;

	acceptThread = thread(AcceptClient, serverContext);
	sendThread = thread(SendAllClient, serverContext);

}

void NetworkServer::CloseListenSocket()
{
	isRunning = false;

	if (listenSock != INVALID_SOCKET) {
		closesocket(listenSock);
		listenSock = INVALID_SOCKET;
	}

	SetEvent(snapshotPacketBuffer.GetSendEvent());

	if (acceptThread.joinable()) 
	{
		acceptThread.join();
	}
	if (sendThread.joinable()) 
	{
		sendThread.join();
	}
}

uint64_t NetworkServer::GetTimestampMs()
{
	using namespace std::chrono;
	return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>
		(steady_clock::now().time_since_epoch()).count();
}

void NetworkServer::SendPacketAllClient()
{
	TickSnapshotPacket tickSnapshotPacket;

	while(snapshotPacketBuffer.TryGetSnapshotPacket(tickSnapshotPacket) == true)
	{
		for (const auto& client : clients)
		{
			if (!client->IsConnected())
			{
				continue;
			}
			client->SendPacket(tickSnapshotPacket);
		}
	}

	LocalMissileEventPacket missileEventPacket;
	while (eventQueue.TryPopPacket(missileEventPacket) == true)
	{
		Client* client = clients[missileEventPacket.playerNum];
		client->SendPacket(missileEventPacket);
	}
}

