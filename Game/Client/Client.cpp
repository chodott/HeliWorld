#include"stdafx.h"
#include "Client.h"
char* CilentName = (char*)"test.mp4";
char* SERVERIP = (char*)"127.0.0.1";
struct headerFile {
    long long sendSize;
    int TitleLength;
};
#pragma pack()

//int main(int argc, char* argv[])
//{

//}

int AcceptClient()
{
	int retval;
	char* SERVERIP = (char*)"127.0.0.1";
	// 명령행 인수가 있으면 IP 주소로 사용
	//if (argc > 1) SERVERIP = argv[1];
	int ServerPort[2] = {};
	// 윈속 초기화
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;
	// 소켓 생성
	SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock == INVALID_SOCKET) err_quit("socket()");
	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(SERVERPORT);
	retval = connect(sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr));
	//if (retval == SOCKET_ERROR)err_quit("socket()");
	// 데이터 통신에 사용할 변수
	closesocket(sock);
}
