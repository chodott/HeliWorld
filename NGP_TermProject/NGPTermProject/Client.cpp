#include "stdafx.h"
#include <thread>
#include "Client.h"

int PacketSizeHelper(char packetType)
{
	int packetSize;
	switch (packetType)
	{
	case PACKET::PlayerInfo:
		packetSize = sizeof(PlayerInfoBundlePacket);
		break;
	case PACKET::ItemInfo:
		packetSize = sizeof(ItemInfoBundlePacket);
		break;
	case PACKET::MissileInfo:
		packetSize = sizeof(MissileInfoBundlePacket);
		break;
	case PACKET::KeyInfo:
		packetSize = sizeof(PlayerKeyPacket);
		break;
	case PACKET::PingpongInfo:
		packetSize = sizeof(PingpongPacket);
		break;
	default:
		packetSize = -1;
		break;
	}
	return packetSize;
}

void Client::PacketProcessHelper(char packetType, char* fillTarget)
{
	switch (packetType)
	{
	case PACKET::PlayerInfo:
	{
		auto& pkt = *reinterpret_cast<const PlayerInfoBundlePacket*>(fillTarget);
		frameDataManager->CombinePacket(pkt);
		break;
	}
	case PACKET::ItemInfo:
	{
		auto& pkt = *reinterpret_cast<const ItemInfoBundlePacket*>(fillTarget);
		frameDataManager->CombinePacket(pkt, networkSyncMgr->GetEstimatedServerTimeMs());
		break;
	}
	case PACKET::MissileInfo:
	{
		auto& pkt = *reinterpret_cast<const MissileInfoBundlePacket*>(fillTarget);
		frameDataManager->CombinePacket(pkt);
		break;
	}
	case PACKET::PingpongInfo:
	{
		auto& pkt = *reinterpret_cast<const PingpongPacket*>(fillTarget);
		ReceivePingPongPacket(pkt);
		break;
	}
	default:
		break;
	}
}

Client::Client()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

	sock = new SOCKET();
}

Client::Client(NetworkSyncManager* networkSyncMgr, FrameDataManager* frameDataMgr):Client()
{
	this->networkSyncMgr = networkSyncMgr;
	this->frameDataManager = frameDataMgr;
}

Client::~Client()
{
	closesocket(*sock);
	WSACleanup();
}

void Client::ConnectServer()
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

	if (recv(*sock, (char*)&PlayerNum, sizeof(int), MSG_WAITALL) == SOCKET_ERROR)
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
	frameDataManager->SetPlayerNum(PlayerNum);

	CreateThread(NULL, 0, ReceiveFromServer, this, 0, NULL);
	CreateThread(NULL, 0, SendPingPacket, this, 0, NULL);
	CreateThread(NULL, 0, SendInputPacket, this, 0, NULL);
}

void Client::KeyDownHandler(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_UP: sendKey |= option0;   break;      //0000 0001 
		case VK_DOWN: sendKey |= option1;   break;      //0000 0010
		case VK_LEFT: sendKey |= option2;   break;      //0000 0100
		case VK_RIGHT: sendKey |= option3;   break;      //0000 1000
		case 'Q': sendKey |= option4;   break;      //0001 0000
		case 'E': sendKey |= option5;   break;      //0010 0000
		case ' ': sendKey |= option6;   break;
		}
	}
}

void Client::KeyUpHandler(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_UP: sendKey &= (~option0);   break;
		case VK_DOWN: sendKey &= (~option1);   break;
		case VK_LEFT: sendKey &= (~option2);   break;
		case VK_RIGHT: sendKey &= (~option3);   break;
		case 'Q': sendKey &= (~option4);   break;
		case 'E': sendKey &= (~option5);   break;
		case ' ': sendKey &= (~option6);   break;
		}
	}
}

void Client::PrepareInputPacket(XMFLOAT3& playerPYR)
{
	std::unique_lock<std::mutex> lock(inputPacketLock);
	uint64_t serverTimestamp = networkSyncMgr->GetEstimatedServerTimeMs();

	if (inputPacket_dq.empty()) inputPacket_dq.emplace_back();
	PlayerKeyPacket& cs_key = inputPacket_dq.back();
	cs_key.playerKeyInput = sendKey;
	cs_key.rotation = playerPYR;
	cs_key.timestamp = serverTimestamp;

	if (prevKey != sendKey)
	{
		cs_key.bKeyChanged = true;
		cs_key.launchedMissileNum = lastLaunchedMissileNum;
		inputChangedCV.notify_one();
	}
	prevKey = sendKey;
	sendKey &= (~option6);
	lastLaunchedMissileNum = -1;
	deltaMouse = { 0.0f, 0.0f };
}

void Client::GetKeyPacketToSend(PlayerKeyPacket& keyPacket)
{
	if (!inputPacket_dq.empty())
	{
		keyPacket = inputPacket_dq.front();
		inputPacket_dq.pop_front();
	}
}

void Client::ReceivePingPongPacket(const PingpongPacket& ppPacket)
{
	networkSyncMgr->UpdateSyncData(ppPacket.clientTimeStamp, ppPacket.serverSendTimeStamp);
}

DWORD WINAPI SendPingPacket(LPVOID arg)
{
	Client* client = (Client*)arg;
	SOCKET* sock = client->GetClientsock();

	PingpongPacket cs_pingpong{PACKET::PingpongInfo};

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

DWORD WINAPI SendInputPacket(LPVOID arg)
{
	Client* client = (Client*)arg;
	SOCKET* sock = client->GetClientsock();

	PlayerKeyPacket cs_keyInput;
	while (true)
	{
		{
			std::unique_lock<std::mutex> lock(client->inputPacketLock);
			client->inputChangedCV.wait_for(lock, std::chrono::milliseconds(33));
			client->GetKeyPacketToSend(cs_keyInput);
		}

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

	const int bufSize = 512;
	char buf[bufSize]{};
	while (true)
	{
		if (recv(*sock, (char*)&buf, bufSize, MSG_WAITALL) == SOCKET_ERROR)		err_quit("recv()");

		int restBufSize = bufSize;
		int bufOffset = 0;
		// process remain packet
		if (client->remainSize > 0)
		{
			memcpy(&client->remain[client->remainOffset], buf, client->remainSize);
			restBufSize -= client->remainSize;
			client->PacketProcessHelper(client->remain[0], client->remain);
			bufOffset += client->remainSize;

			// reset remain
			memset(client->remain, 0, 512);
			client->remainOffset = 0;
			client->remainSize = 0;
		}

		while (restBufSize > 0)
		{
			char packetType = buf[bufOffset];
			int packetSize = PacketSizeHelper(packetType);

			// save remain packet
			if (restBufSize < packetSize)
			{
				client->remainOffset = restBufSize;
				client->remainSize = packetSize - client->remainOffset;
				memcpy(&client->remain, buf + bufOffset, restBufSize);
				restBufSize -= client->remainSize;
				break;
			}

			// Packet process
			client->PacketProcessHelper(packetType, buf + bufOffset);
			restBufSize -= packetSize;
			bufOffset += packetSize;
		}
	}
}
