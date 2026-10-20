#include "Client.h"
#include "PacketReceiveBuffer.h"

DWORD WINAPI ReceiveFromClient(LPVOID arg)
{
	ReceiveClientContext* receiveClientContext = static_cast<ReceiveClientContext*>(arg);

	NetworkServer* netServer = receiveClientContext->serverContext->netServer;
	SimulationServer* simServer = receiveClientContext->serverContext->simServer;


	InputManager* inputManager = receiveClientContext->serverContext->inputManager;
	Client* client = receiveClientContext->client;
	int playerNumber = client->GetPlayerNumber();

	simServer->AccessNewPlayer(playerNumber);

	TimebasePacket timebasePacket{ playerNumber, simServer->GetTick(), netServer->GetTimestampMs() };
	if (!client->SendPacket(timebasePacket))
	{
		client->RequestDisconnect();
		return 0;
	}

	PacketReceiveBuffer receiveBuffer;
	const SOCKET receiveSocket = client->GetSocket();
	while (netServer->IsRunning() && client->IsConnected())
	{
		const int freeSize = receiveBuffer.GetFreeSize();
		if (freeSize <= 0)
			break;

		const int receivedBytes = recv(receiveSocket, receiveBuffer.GetWriteBuffer(), freeSize, 0);
		if (receivedBytes <= 0)
			break; // Both orderly close (0) and socket errors end this connection.

		const bool valid = receiveBuffer.ProcessReceived(receivedBytes,
			[&](char packetType, const char* data, int packetSize)
		{
			switch (packetType)
			{
			case CS_KeyInfo:
			{
				PlayerInputPacket inputPacket{};
				memcpy(&inputPacket, data, packetSize);
				// T는 마지막 완료 틱이다. 최근 6틱과 최대 2틱 앞선 입력만 받는다.
				const uint64_t lastCompletedTick = simServer->GetTick();
				if (Protocol::IsInputTickAllowed(inputPacket.estimatedTick, lastCompletedTick))
				{
					inputManager->PushInputData(playerNumber, inputPacket);
				}
				break;
			}
			case CS_PingpongInfo:
			{
				PingpongPacket ppPacket{};
				memcpy(&ppPacket, data, packetSize);
				ppPacket.serverSendTimeStamp = netServer->GetTimestampMs();
				return client->SendPacket(ppPacket);
			}
			}
			return true;
		});
		if (!valid)
			break;
	}

	// The send/accept thread performs shutdown, join and close.
	client->RequestDisconnect();
	return 0;
}

Client::~Client()
{
	Disconnect();
}

void Client::Connect(const SOCKET clientSock, const int playerNum, ServerContext* serverContext)
{
	std::lock_guard<std::mutex> lock(connectionLock);
	CloseConnection();

	sock = clientSock;
	SetPlayerNumber(playerNum);

	recvCtx.serverContext = serverContext;
	recvCtx.client = this;
	serverContext->inputManager->ClearPlayerInputs(playerNum);
	shouldDisconnected = false;
	m_connected = true;

	recvThread = thread(ReceiveFromClient, &recvCtx);

	std::cout << "Client accepted in " << GetPlayerNumber() << std::endl;
}

void Client::Disconnect()
{
	std::lock_guard<std::mutex> lock(connectionLock);
	CloseConnection();
}

void Client::RequestDisconnect()
{
	shouldDisconnected = true;
	// Wake the owner even when no simulation snapshot is being produced.
	if (recvCtx.serverContext && recvCtx.serverContext->snapshotBuffer)
		SetEvent(recvCtx.serverContext->snapshotBuffer->GetSendEvent());
}

void Client::CleanupDisconnected()
{
	std::lock_guard<std::mutex> lock(connectionLock);
	if (shouldDisconnected.load())
		CloseConnection();
}

void Client::CloseConnection()
{
	m_connected = false;
	const SOCKET closingSocket = sock.load();
	// Stop further sends, then close to cancel a blocking recv before joining.
	if (closingSocket != INVALID_SOCKET)
		shutdown(closingSocket, SD_BOTH);
	{
		std::lock_guard<std::mutex> lock(sendLock);
		if (closingSocket != INVALID_SOCKET)
			closesocket(closingSocket);
		sock = INVALID_SOCKET;
	}

	if (recvThread.joinable())
		recvThread.join();
	// No old receiver can enqueue input after the slot has been cleared.
	if (recvCtx.serverContext && m_playerNumber >= 0)
		recvCtx.serverContext->inputManager->ClearPlayerInputs(m_playerNumber);
	shouldDisconnected = false;
	lastLaunchedMissileID = 0;
}

bool Client::SendBytes(const char* data, int size)
{
	std::lock_guard<std::mutex> lock(sendLock);
	if (!IsConnected())
		return false;

	int sentSize = 0;
	while (sentSize < size)
	{
		const int result = send(sock.load(), data + sentSize, size - sentSize, 0);
		if (result <= 0)
		{
			RequestDisconnect();
			return false;
		}
		sentSize += result;
	}
	return true;
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

