#include "Client.h"

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

DWORD WINAPI ReceiveFromClient(LPVOID arg)
{
	ReceiveClientContext* receiveClientContext = static_cast<ReceiveClientContext*>(arg);
	NetworkServer* networkServer = receiveClientContext->serverContext->netServer;
	SimulationServer* simulationServer = receiveClientContext->serverContext->simServer;
	ClientInputBuffer* inputBuffer = receiveClientContext->serverContext->inputBuffer;

	//Init Packet
	Client* client = (Client*)arg;
	int playerNumber = client->GetPlayerNumber();
	TimebasePacket timebasePacket{ playerNumber, simulationServer->GetTick(), networkServer->GetTimestampMs() };
	send(client->GetSocket(), (char*)&timebasePacket, sizeof(timebasePacket), 0);

	client->Connected();
	PlayerKeyPacket keyPacket{};

	int combinedSize = 0;
	char buf[BUFSIZE]{};
	while (true)
	{
		int receivedBytes = recv(client->GetSocket(), (char*)&buf, BUFSIZE, 0);
		if (receivedBytes == SOCKET_ERROR)
		{
			// cut the connection
			client->Reset();
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
				memcpy(&keyPacket, client->remainBuffer + offset, packetSize);
				inputBuffer->PushInputData(client->GetPlayerNumber(), simulationServer->GetTick(), keyPacket);
				break;
			}
			case CS_PingpongInfo:
			{
				PingpongPacket ppPacket;
				memcpy(&ppPacket, client->remainBuffer + offset, packetSize);
				ppPacket.serverSendTimeStamp = networkServer->GetTimestampMs();
				send(client->GetSocket(), (char*)&ppPacket, PacketSizeHelper(CS_PingpongInfo), 0);
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

void Client::Connect(const SOCKET clientSock, const int playerNum, ServerContext* serverContext)
{
	sock = clientSock;
	SetPlayerNumber(playerNum);

	recvCtx.serverContext = serverContext;
	recvCtx.client = this;

	recvHandle = CreateThread(NULL, 0, ReceiveFromClient, &recvCtx, 0, NULL);
	std::cout << "Client accepted in " << GetPlayerNumber() << std::endl;
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
}

