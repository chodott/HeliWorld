#pragma once
#pragma warning(disable : 26495)

#include "stdafx.h"

const char SC_PlayerInfo = 0;
const char SC_PlayerStatus = 1;
const char SC_PlayerKey = 2;


#pragma pack(1)
struct PlayerInfoPacket {
	char packetType;
	int playerNumber;
	XMFLOAT3 movement;
	XMFLOAT3 rotation;
};

struct PlayerStatusPacket {
	char packetType;
	int playerNumber;
	unsigned char activatedMissiles;
	int playerHP;
};

struct PlayerKeyPacket {
	char playerKeyInput;
	POINT mousePosition;
};

struct ItemInfoPacket {
	char packetType;
	float x, y, z;
	bool active;
};

struct MissileInfoPacket {
	char packetType;
	int playerNumber;
	XMFLOAT3 movement;
	XMFLOAT3 rotation;
};
#pragma pack()

