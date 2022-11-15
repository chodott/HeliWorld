#pragma once

#include "error.h"
#include <unordered_map>

//키 매크로 선언
const char KEY_W = 0x01;
const char KEY_A = 0x02;
const char KEY_S = 0x04;
const char MOUSE_LEFT = 0x10;
const char MOUSE_RIGHT = 0x20;

int RecieveServer();

DWORD WINAPI ReceiveAllClient(LPVOID arg)