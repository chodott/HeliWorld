#include "stdafx.h"
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
	default:
		packetSize = -1;
		break;
	}
	return packetSize;
}

void PacketProcessHelper(char packetType, char* fillTarget, Client* client)
{
	switch (packetType)
	{
	case PACKET::PlayerInfo:
	{
		PlayerInfoBundlePacket piPacket;
		memcpy(&piPacket, fillTarget, sizeof(PlayerInfoBundlePacket));
		memcpy(&client->playerData, &piPacket.playerInfos, sizeof(PlayerInfoPacket) * 4);
		break;
	}
	case PACKET::ItemInfo:
	{
		ItemInfoBundlePacket itPacket;
		memcpy(&itPacket, fillTarget, sizeof(ItemInfoBundlePacket));
		memcpy(&client->itemPacket, itPacket.itemInfos, sizeof(ItemInfoPacket)*10);
		break;
	}
	case PACKET::MissileInfo:
	{
		MissileInfoBundlePacket miPacket;
		memcpy(&miPacket, fillTarget, sizeof(MissileInfoBundlePacket));
		memcpy(&client->missilePacket, miPacket.missileInfos, sizeof(MissileInfoPacket) * 32);
		break;
	}
	case PACKET::KeyInfo:
	{
		PlayerKeyPacket pkPacket;
		memcpy(&pkPacket, fillTarget, sizeof(PlayerKeyPacket));
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

	playerData[PlayerNum].playerNumber = PlayerNum;

	CreateThread(NULL, 0, ReceiveFromServer, this, 0, NULL);
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

void Client::SendtoServer()
{
	PlayerKeyPacket cs_key;
	cs_key.packetType = PACKET::KeyInfo;
	cs_key.playerKeyInput = sendKey;
	cs_key.deltaMouse = deltaMouse;
	cs_key.launchedMissileNum = lastLaunchedMissileNum;
	if(lastLaunchedMissileNum >=0) cout << lastLaunchedMissileNum << " ";
	if (send(*sock, (char*)&cs_key, sizeof(PlayerKeyPacket), 0) == SOCKET_ERROR)
	{
		err_display("send()");
		return;
	}
	sendKey &= (~option6);
	lastLaunchedMissileNum = -1;
}

uint32_t Client::GetTimestampMs()
{
	using namespace std::chrono;
	return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>
		(steady_clock::now().time_since_epoch()).count();
}

DWORD WINAPI ReceiveFromServer(LPVOID arg)
{
	Client* client = (Client*)arg;
	SOCKET* sock = client->GetClientsock();

	const int bufSize = 512;
	char buf[bufSize]{};
	while (true)
	{
		const int frameTime = 17;		// 1000ms / 60frame	+ 1
		if (recv(*sock, (char*)&buf, bufSize, MSG_WAITALL) == SOCKET_ERROR)		err_quit("recv()");

		int restBufSize = bufSize;
		int bufOffset = 0;
		// process remain packet
		if (client->remainSize > 0)
		{
			memcpy(&client->remain[client->remainOffset], buf, client->remainSize);
			restBufSize -= client->remainSize;
			PacketProcessHelper(client->remain[0], client->remain, client);
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
			PacketProcessHelper(packetType, buf + bufOffset, client);
			restBufSize -= packetSize;
			bufOffset += packetSize;
		}
	}
}
