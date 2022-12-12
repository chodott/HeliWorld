#pragma once
//#include"stdafx.h"
#include "DirectXMath.h"

namespace PACKET {
	const char PlayerInfo = 0;
	//const char PlayerKey = 1;
	const char ItemInfo = 1;
	const char MissileInfo = 2;
};


struct FPoint {
	float x;
	float y;
};


#pragma pack(1)
struct PlayerInfoPacket {
	char packetType;
	int playerNumber = -1;
	int playerHP;
	XMFLOAT3 movement;
	XMFLOAT3 rotation;
	bool playerActive;
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
	int itemNum;
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

