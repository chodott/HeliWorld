#pragma once

#include "error.h"
#include <unordered_map>

//플레이어 정보 관리 컨테이너	*value값은 임의로 int 선언 - 변경 필요>

int RecieveServer();

DWORD WINAPI ReceiveAllClient(LPVOID arg);