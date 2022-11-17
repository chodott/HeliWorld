
#pragma once
#include"stdafx.h"

const char SC_PlayerInfo = 0;
const char SC_PlayerStatus = 1;
const char SC_PlayerKey = 2;


#pragma pack(1)
struct PlayerInfoPacket {
    char packetType;
    char playerNumber;
    XMFLOAT3 movement;
    XMFLOAT3 rotation;
};

struct PlayerStatusPacket {
	char packetType;
	char playerNumber;
	char activatedMissiles;
	int playerHP;
};

struct PlayerKeyPacket {
	char playerKeyInput;
	POINT mousePosition;

};
#pragma pack()