#pragma once
#pragma warning(disable : 26495)

#include "stdafx.h"

const char SC_PlayerInfo = 0;
const char SC_PlayerStatus = 1;
const char SC_PlayerKey = 2;
const char SC_ItemInfo = 3;
const char SC_MissileInfo = 4;


struct FPoint {
	float x;
	float y;
};


#pragma pack(1)
struct PlayerInfoPacket {
	char packetType;
	int playerNumber;
	XMFLOAT3 movement;
	XMFLOAT3X3 rotationMatrix;
};
#pragma pack()

#pragma pack(1)
struct PlayerStatusPacket {
	char packetType;
	int playerNumber;
	unsigned char activatedMissiles;
	int playerHP;
};
#pragma pack()

#pragma pack(1)
struct PlayerKeyPacket {
	char playerKeyInput;
	FPoint deltaMouse;
};
#pragma pack()

#pragma pack(1)
struct ItemInfoPacket {
	char packetType;
	float x, y, z;
	bool active;
};
#pragma pack()

#pragma pack(1)
struct MissileInfoPacket {
	char packetType;
	int playerNumber;
	bool active = false;
	XMFLOAT3 movement;
	XMFLOAT3 rotation;
};
#pragma pack()

