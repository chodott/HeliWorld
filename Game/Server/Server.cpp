#include"stdafx.h"
#include "Server.h"
#define SERVERPORT 9000
#define BUFSIZE    512

//플레이어 정보 관리 컨테이너	*value값은 임의로 int 선언 - 변경 필요>
std::unordered_map<SOCKET, int> PlayerDataMap;

int RecieveServer()
{
	int retval;

	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// 소켓 생성
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

	// 데이터 통신에 사용할 변수
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

		// 접속한 클라이언트 정보 출력
		char addr[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));
		printf_s("\n[TCP 서버] 클라이언트 접속: IP 주소=%s, 포트 번호=%d\n",
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
			printf_s("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);

			// 데이터 보내기
			retval = send(client_sock, buf, retval, 0);
			if (retval == SOCKET_ERROR) {
				err_display("send()");
				break;
			}
		}

		// 소켓 닫기
		closesocket(client_sock);
		printf_s("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
			addr, ntohs(clientaddr.sin_port));
	}

	// 소켓 닫기
	closesocket(listen_sock);

	// 윈속 종료
	WSACleanup();
	return 0;
}

DWORD WINAPI ReceiveAllClient(LPVOID arg)
{
	//소켓 정보 저장할 곳 필요
	int retval;
	sockaddr_in clientaddr;
	char addr[INET_ADDRSTRLEN];
	char buf[BUFSIZE + 1];
	char PlayerInput[4];


	// 클라이언트와 데이터 통신
	while (1) {
		int cnt = 0;
		for (auto PlayerData: PlayerDataMap){	//플레이어 접속 수만큼 순환
			// 데이터 받기
			retval = recv(PlayerData.first, buf, BUFSIZE, 0);
			if (retval == SOCKET_ERROR) {
				err_display("recv()");
				break;
			}

			//키 on/off 확인
			PlayerInput[cnt] = buf[0];
			cnt++;
			// 받은 데이터 출력
			//buf[retval] = '\0';
			//printf("[TCP/%s:%d] %s\n", addr, ntohs(clientaddr.sin_port), buf);
			}
		}

	return 1;
}
