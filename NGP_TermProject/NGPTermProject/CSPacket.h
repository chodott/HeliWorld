#pragma once
//#include"stdafx.h"
#include "DirectXMath.h"

namespace PACKET {
	const char PlayerInfo = 0;
	const char PlayerStatus = 1;
	const char PlayerKey = 2;
	const char ItemInfo = 3;
	const char MissileInfo = 4;
};


struct FPoint {
	float x;
	float y;
};


#pragma pack(1)
struct PlayerInfoPacket {
	char packetType;
	int playerNumber = -1;
	XMFLOAT3 movement;
	XMFLOAT3X3 rotationMatrix;
};
#pragma pack()

#pragma pack(1)
struct PlayerStatusPacket {
	char packetType;
	int playerNumber;
	unsigned char activatedMissiles = 0;
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
	XMFLOAT3 Position;
	bool active;
};
#pragma pack()

#pragma pack(1)
struct MissileInfoPacket {
	char packetType;
	int playerNumber;		
	int missileNumber;
	bool active = false;
	XMFLOAT3 movement;
	XMFLOAT3 rotation;
};
#pragma pack()

