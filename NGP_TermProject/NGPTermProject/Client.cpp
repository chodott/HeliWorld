#include "stdafx.h"
#include <thread>
#include "Client.h"
#include <fstream>
#include <filesystem>

int PacketSizeHelper(char packetType)
{
	int packetSize;
	switch (packetType)
	{
	case PACKET::SnapshotInfo:
		packetSize = sizeof(TickSnapshotPacket);
		break;
	case PACKET::KeyInfo:
		packetSize = sizeof(PlayerKeyPacket);
		break;
	case PACKET::PingpongInfo:
		packetSize = sizeof(PingpongPacket);
		break;
	case PACKET::LocalMissileEvent:
		packetSize = sizeof(LocalMissileEventPacket);
		break;
	default:
		cout << "wrong packet type" << "\n";
		packetSize = -1;
		break;
	}
	return packetSize;
}

void Client::PacketProcessHelper(char packetType, char* buffer)
{
	for (auto& packetListener : packetListner_vec)
	{
		packetListener->OnReceivePacket(packetType, buffer);
	}
}

Client::Client()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	sock = new SOCKET();
}

Client::~Client()
{
	closesocket(*sock);
	WSACleanup();
}

void Client::ConnectServer(InitDataPacket& initData)
{

	if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)	err_quit("socket()");

	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, serverIp, &serverAddr.sin_addr);
	serverAddr.sin_port = htons(SERVERPORT);

	DWORD recvTimeout = 5000;		// 5000ms
	int errorCode = setsockopt(*sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&recvTimeout, sizeof(recvTimeout));
	setsockopt(*sock, IPPROTO_TCP, TCP_NODELAY, (char*)&recvTimeout, sizeof(recvTimeout));

	if (connect(*sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		err_quit("socket()");
	}
	std::cout << "Socket initalize successful\n";

	if (recv(*sock, (char*)&initData, sizeof(InitDataPacket), MSG_WAITALL) == SOCKET_ERROR)
	{
		if (WSAGetLastError() == WSAETIMEDOUT)
		{
			cout << "The room is full" << endl;
			system("pause");
			exit(1);
		}
		else
		{
			err_quit("socket()");
		}
	}

	recvThread = thread(ReceiveFromServer, 0);
	sendPingThread = thread(SendPingToServer, 0);
	sendInputThread = thread(SendInputToServer, 0);
}


void Client::GetKeyPacketToSend(PlayerKeyPacket& keyPacket)
{
	std::unique_lock<std::mutex> lock(inputPacketLock);
	if (!inputPacket_dq.empty())
	{
		keyPacket = inputPacket_dq.front();
		inputPacket_dq.pop_front();
	}
}

void Client::AddKeyPacket(const PlayerKeyPacket& keyPacket)
{
	std::unique_lock<std::mutex> lock(inputPacketLock);
	inputPacket_dq.push_back(keyPacket);
}

DWORD WINAPI SendPingToServer(LPVOID arg)
{
	Client* client = (Client*)arg;
	SOCKET* sock = client->GetClientsock();

	PingpongPacket cs_pingpong{ PACKET::PingpongInfo };

	while (true)
	{
		cs_pingpong.clientTimeStamp = client->networkSyncMgr->GetTimestampMs();
		if (send(*sock, (char*)&cs_pingpong, sizeof(PingpongPacket), 0) == SOCKET_ERROR)
		{
			err_display("send()");
			return 0;
		}

		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

DWORD WINAPI SendInputToServer(LPVOID arg)
{
	Client* client = (Client*)arg;
	SOCKET* sock = client->GetClientsock();

	PlayerKeyPacket cs_keyInput;
	while (true)
	{
		client->GetKeyPacketToSend(cs_keyInput);

		if (send(*sock, (char*)&cs_keyInput, sizeof(PlayerKeyPacket), 0) == SOCKET_ERROR)
		{
			err_display("send()");
			return 0;
		}

	}
}

DWORD WINAPI ReceiveFromServer(LPVOID arg)
{
	Client* client = (Client*)arg;
	SOCKET* sock = client->GetClientsock();

	int combinedSize = 0;
	char buf[BUFSIZE]{};
	while (true)
	{
		int receivedBytes = recv(*sock, buf, BUFSIZE, 0);

		if (receivedBytes == SOCKET_ERROR) {
			int err = WSAGetLastError();
			std::cout << "recv error: " << err << "\n";
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
			if (packetSize < 0) {
				offset += 1;
				continue;
			}

			if (offset + packetSize > combinedSize)
			{
				//wait next packet
				break;
			}

			// Packet process
			client->PacketProcessHelper(packetType, client->remainBuffer + offset);

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
