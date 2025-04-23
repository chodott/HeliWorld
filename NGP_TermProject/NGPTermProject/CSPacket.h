#pragma once
//#include"stdafx.h"
#include "DirectXMath.h"

namespace PACKET {
	const char PlayerInfo = 0;
	const char ItemInfo = 1;
	const char MissileInfo = 2;
};


struct FPoint {
	float x;
	float y;
};


#pragma pack(1)
struct PlayerInfoPacket {
	int playerNumber = -1;
	int playerHP;
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	bool playerActive;
};

struct PlayerInfoBundlePacket
{
	char packetType;
	PlayerInfoPacket playerInfos[4];
	uint32_t serverTimestampMs;
};

struct PlayerKeyPacket {
	char playerKeyInput;
	FPoint deltaMouse;
};

struct ItemInfoPacket {
	char packetType;
	int itemNum;
	XMFLOAT3 position;
	bool active;
};

struct MissileInfoPacket {
	char packetType;
	int playerNumber;		
	int missileNumber;
	bool active = false;
	XMFLOAT3 position;
};
#pragma pack()

