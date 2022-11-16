//#include "stdafx.h"
//#include <WS2tcpip.h>
#include "Server.h"


//#include "Socket.h"

//#pragma lib(lib, "ws2_32")

Server::Server()
{
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);
}

Server::~Server()
{
	WSACleanup();
}

void Server::OpenListenSocket()
{
	// 소켓 생성
	listenSock = socket(AF_INET, SOCK_STREAM, 0);
	if (listenSock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	//retval = bind(listenSock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (bind(listenSock, (sockaddr*)&serveraddr, sizeof(serveraddr)) == SOCKET_ERROR)
		err_quit("bind()");
	//if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	if (listen(listenSock, SOMAXCONN) == SOCKET_ERROR)
		err_quit("listen()");
	//if (retval == SOCKET_ERROR) err_quit("listen()");
}

void Server::SendAllClient()
{
	for (int i = 0; i < clientNum; ++i)
	{
		for (int j = 0; i < clientNum; ++i)
		{
			if (clients.at(i) != nullptr)
				send(clients.at(i)->clientSock, (char*)infoPackets.at(i), sizeof(PlayerInfoPacket), 0);
				// need conversion operator packet => char* 
		}
	}
}

int ReceiveServer()
{
	int retval;

	//// 윈속 초기화
	//WSADATA wsa;
	//if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	//	return 1;

	//// 소켓 생성
	//SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	//if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	//// bind()
	//struct sockaddr_in serveraddr;
	//memset(&serveraddr, 0, sizeof(serveraddr));
	//serveraddr.sin_family = AF_INET;
	//serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	//serveraddr.sin_port = htons(SERVERPORT);
	//retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	//if (retval == SOCKET_ERROR) err_quit("bind()");

	//// listen()
	//retval = listen(listen_sock, SOMAXCONN);
	//if (retval == SOCKET_ERROR) err_quit("listen()");

	// 데이터 통신에 사용할 변수



	SOCKET client_sock;
	struct sockaddr_in clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(*g_server.GetSocket(), (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));

		// 클라이언트와 데이터 통신
		while (1) {
			// 데이터 받기
			retval = recv(client_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// 받은 데이터 출력
			buf[retval] = '\0';
			printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);

			// 데이터 보내기
			retval = send(client_sock, buf, retval, 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
		}

		// 소켓 닫기
		closesocket(client_sock);
		printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}

	// 소켓 닫기
	closesocket(*g_server.GetSocket());

	// 윈속 종료
	WSACleanup();
	return 0;
}


