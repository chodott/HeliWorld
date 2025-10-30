#pragma once
#include"stdafx.h"
#include "DirectXMath.h"

namespace PACKET {
	const char PlayerInfo = 0;
	const char ItemInfo = 1;
	const char MissileInfo = 2;
	const char KeyInfo = 3;
	const char PingpongInfo = 4;
	const char LocalMissileEvent = 5;

};


struct FPoint {
	float x;
	float y;
};


#pragma pack(1)
struct InitDataPacket
{
	int playerNum;
	uint64_t serverTick;
	uint64_t serverTimestamp;
};

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
	int serverTick;
};


struct PlayerKeyPacket 
{
	char packetType = PACKET::KeyInfo;
	unsigned char playerKeyInput;
	XMFLOAT3 rotation;
	int16_t launchedMissileNum = -1;
	uint64_t estimatedTick;
	bool bKeyChanged = false;
};

struct ItemInfoPacket {
	int32_t positionX;
	int32_t positionY;
	int32_t positionZ;
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

struct LocalMissileEventPacket
{
	char packetType;
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	uint64_t missileNum;
	uint64_t eventTick;
	int playerNum;
	bool active;
};

#pragma pack()

