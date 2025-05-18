#pragma once
#include"stdafx.h"
#include "DirectXMath.h"

namespace PACKET {
	const char PlayerInfo = 0;
	const char ItemInfo = 1;
	const char MissileInfo = 2;
	const char KeyInfo = 3;
	const char PingpongInfo = 4;
};


struct FPoint {
	float x;
	float y;
};


#pragma pack(1)
struct PingpongPacket
{
	char packetType;
	uint64_t clientTimeStamp;
	uint64_t serverSendTimeStamp;
};


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
	uint64_t timestamp;
};


struct PlayerKeyPacket {
	char packetType;
	unsigned char playerKeyInput;
	FPoint deltaMouse;
	int16_t launchedMissileNum = -1;
	uint64_t timestamp;
	bool bKeyChanged = false;
};

struct ItemInfoPacket {
	int16_t positionX;
	int16_t positionY;
	int16_t positionZ;
	bool active;
};

struct ItemInfoBundlePacket
{
	char packetType;
	ItemInfoPacket itemInfos[10];
};

struct MissileInfoPacket {

	bool active;
	int16_t positionX;  
	int16_t positionY; 
	int16_t positionZ; 
};

struct MissileInfoBundlePacket
{
	char packetType;
	MissileInfoPacket missileInfos[32];
};

#pragma pack()

