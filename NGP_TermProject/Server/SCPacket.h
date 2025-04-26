#pragma once
#pragma warning(disable : 26495)

#include "stdafx.h"

const char SC_PlayerInfo = 0;
const char SC_ItemInfo = 1;
const char SC_MissileInfo = 2;
const char SC_KeyInfo = 3;


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
	char packetType;
	unsigned char playerKeyInput;
	FPoint deltaMouse;
	uint32_t timestampMs;
};

struct ItemInfoPacket {
	XMFLOAT3 position;
	bool active;
};

struct ItemInfoBundlePacket
{
	char packetType;
	ItemInfoPacket itemInfos[10];
};

struct MissileInfoPacket {
	bool active;
	XMFLOAT3 position;
};

struct MissileInfoBundlePacket
{
	char packetType;
	MissileInfoPacket missileInfos[32];
};

#pragma pack()

