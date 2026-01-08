#pragma once
#include"stdafx.h"
#include "DirectXMath.h"
#include "ProtocolConstants.h"

namespace PACKET 
{
	const char SnapshotInfo = 1;
	const char KeyInfo = 3;
	const char PingpongInfo = 4;
	const char LocalMissileEvent = 5;

};


struct FPoint 
{
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


struct PlayerInfoPacket 
{
	int playerNumber = -1;
	int playerHP;
	XMFLOAT3 position;
	XMFLOAT3 rotation;
	bool playerActive;
};

struct PlayerKeyPacket 
{
	char packetType = PACKET::KeyInfo;
	unsigned char playerKeyInput;
	XMFLOAT3 rotation;
	uint64_t launchedMissileNum = 0;
	uint64_t estimatedTick;
};

struct ItemInfoPacket 
{
	int32_t positionX;
	int32_t positionY;
	int32_t positionZ;
	bool active;
};

struct MissileInfoPacket {

	bool active;
	XMFLOAT3 position;
};

struct TickSnapshotPacket
{
	char packetType;
	uint64_t serverTick;
	PlayerInfoPacket playerInfos[Protocol::kMaxPlayerCount];
	MissileInfoPacket missileInfos[Protocol::kMaxMissileCount];
	ItemInfoPacket itemInfos[Protocol::kMaxItemCount];
};


struct LocalMissileEventPacket
{
	char packetType;
	uint64_t missileNum;
	uint64_t eventTick;
	int playerNum;
	bool active;
};

#pragma pack()

