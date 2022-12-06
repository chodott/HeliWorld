#include "stdafx.h"
#include "Client.h"

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
	if ((*sock = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		err_quit("socket()");

	sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serverAddr.sin_addr);
	serverAddr.sin_port = htons(SERVERPORT);

	std::cout << "Socket initalize successful\n";

	if (connect(*sock, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
		err_quit("socket()");

	if (recv(*sock, (char*)&PlayerNum, sizeof(int), MSG_WAITALL) == SOCKET_ERROR)
	{
		err_quit("socket()");
	}
	playerData[PlayerNum].playerNumber = PlayerNum;


	CreateThread(NULL, 0, ReceiveFromServer, this, 0, NULL);


	DWORD optval = 1;
	setsockopt(*sock, IPPROTO_TCP, TCP_NODELAY, (const char*)&optval, sizeof(optval));
}

void Client::KeyDownHandler(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'W': sendKey |= option0;	break;      //0000 0001 
		case 'S': sendKey |= option1;	break;      //0000 0010
		case 'A': sendKey |= option2;	break;      //0000 0100
		case 'D': sendKey |= option3;	break;      //0000 1000
		case 'Q': sendKey |= option4;	break;      //0001 0000
		case 'E': sendKey |= option5;	break;      //0010 0000
		case ' ': sendKey |= option6;	break;
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
		case 'W': sendKey &= (~option0);	break;
		case 'S': sendKey &= (~option1);	break;
		case 'A': sendKey &= (~option2);	break;
		case 'D': sendKey &= (~option3);	break;
		case 'Q': sendKey &= (~option4);	break;
		case 'E': sendKey &= (~option5);	break;
		case ' ': sendKey &= (~option6);	break;
		}
	}
}

void Client::SendtoServer()
{
	PlayerKeyPacket cs_key;
	cs_key.playerKeyInput = sendKey;
	cs_key.mousePosition = deltaMouse;
	if (send(*sock, (char*)&cs_key, sizeof(PlayerKeyPacket), 0) == SOCKET_ERROR)
	{
		err_display("send()");
		return;
	}
	//buf[0] = (char)&ptCursorPos;
}

DWORD WINAPI ReceiveFromServer(LPVOID arg)
{
	Client* client = (Client*)arg;
	SOCKET* sock = client->GetClientsock();
	char buf[sizeof(PlayerInfoPacket)]{};
	while (true)
	{
		ResetEvent(client->ReceiveDone);

		if (recv(*sock, (char*)&buf, sizeof(PlayerInfoPacket), MSG_WAITALL) == SOCKET_ERROR)
			err_quit("recv()");

		static int count;
		std::cout << count++ << std::endl;

		switch (buf[0])
		{
		case PACKET::PlayerInfo:
		{
			PlayerInfoPacket piPacket;
			memcpy(&piPacket, buf, sizeof(piPacket));
			client->playerData[piPacket.playerNumber] = piPacket;
			break;
		}
		case PACKET::PlayerStatus:
		{
			PlayerStatusPacket psPacket;
			memcpy(&psPacket, buf, sizeof(psPacket));
			client->playerStatus[psPacket.playerNumber] = psPacket;
			break;
		}
		case PACKET::ItemInfo:

			break;
		case PACKET::MissileInfo:
		{
			MissileInfoPacket miPacket;
			for (int i = 0; i < 32; ++i)
			{
				if (!client->missilePacket[i].active)
					client->missilePacket[i] = miPacket;
				// some where need a garbage collector which clean up missile deactivated
			}
			break;
		}
		default:
			break;
		}

		//if (recv(*sock, (char*)&piPacket, sizeof(PlayerInfoPacket), MSG_WAITALL) == SOCKET_ERROR)
		//	err_quit("pi packet");
		//client->playerData[piPacket.playerNumber] = piPacket;   //->Player and otherPlayer render ->goto Scene.cpp render() and Player.cpp render()

		//PlayerStatusPacket psPacket;
		//if (recv(*sock, (char*)&psPacket, sizeof(PlayerStatusPacket), MSG_WAITALL) == SOCKET_ERROR)
		//	err_quit("ps packet");
		//client->playerStatus[psPacket.playerNumber] = psPacket;

		//MissileInfoPacket miPacket;
		//for (int i = 0; i < 32; ++i)
		//{
		//	if (recv(*sock, (char*)&miPacket, sizeof(MissileInfoPacket), MSG_WAITALL) == SOCKET_ERROR)
		//		err_quit("mi packet");
		//	client->missilePacket[i] = miPacket;
		//}


		SetEvent(client->ReceiveDone);
	}
}

