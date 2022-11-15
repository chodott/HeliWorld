//#include "stdafx.h"
//#include <WS2tcpip.h>
#include "Server.h"


//#include "Socket.h"

//#pragma lib(lib, "ws2_32")

int ReceiveServer()
{
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
	SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_sock == INVALID_SOCKET) err_quit("socket()");

	// bind()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = bind(listen_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	if (retval == SOCKET_ERROR) err_quit("bind()");

	// listen()
	retval = listen(listen_sock, SOMAXCONN);
	if (retval == SOCKET_ERROR) err_quit("listen()");

	// ������ ��ſ� ����� ����
	SOCKET client_sock;
	HANDLE ReceiveThread;
	struct sockaddr_in clientaddr;
	int addrlen;
	char buf[BUFSIZE + 1];

	//Create ReceiveAllClient Thread
	ReceiveThread = CreateThread(NULL, 0, ReceiveAllClient,
		NULL, 0, NULL);
	CloseHandle(ReceiveThread);

	while (1) {
		// accept()
		addrlen = sizeof(clientaddr);
		client_sock = accept(listen_sock, (struct sockaddr*)&clientaddr, &addrlen);
		if (client_sock == INVALID_SOCKET) {
			err_display("accept()");
			break;
		}

		// ������ Ŭ���̾�Ʈ ���� ���
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf_s("\n[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));

		// Ŭ���̾�Ʈ�� ������ ���
		while (1) {
			// ������ �ޱ�
			retval = recv(client_sock, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}
			else if (retval == 0)
				break;

			// ���� ������ ���
			buf[retval] = '\0';
			printf_s("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);

			// ������ ������
			retval = send(client_sock, buf, retval, 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
		}

		// ���� �ݱ�
		closesocket(client_sock);
		printf_s("[TCP ����] Ŭ���̾�Ʈ ����: IP �ּ�=%s, ��Ʈ ��ȣ=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}

	// ���� �ݱ�
	closesocket(listen_sock);

	// ���� ����
	WSACleanup();
	return 0;
}

DWORD WINAPI ReceiveAllClient(LPVOID arg)
{
	//���� ���� ������ �� �ʿ�
	int retval;
	sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	char buf[BUFSIZE + 1];
	char PlayerInput[4];


	// Ŭ���̾�Ʈ�� ������ ���
	while (1) {
		int cnt = 0;
		for (auto PlayerData: PlayerDataMap){	//�÷��̾� ���� ����ŭ ��ȯ
			// ������ �ޱ�
			retval = recv(PlayerData.first, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}

			//Ű on/off Ȯ��
			PlayerInput[cnt] = buf[0];
			cnt++;
			// ���� ������ ���
			//buf[retval] = '\0';
			//printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);
			}
		}

	return 1;
}
