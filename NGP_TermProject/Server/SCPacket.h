#pragma once
#pragma warning(disable : 26495)

#include "stdafx.h"

const char SC_PlayerInfo = 0;
const char SC_ItemInfo = 1;
const char SC_MissileInfo = 2;
const char CS_KeyInfo = 3;
const char CS_PingpongInfo = 4;
const char CS_LocalMissileEvent = 5;

struct FPoint {
	float x;
	float y;
}; 

#pragma pack(1)
struct TimebasePacket
{
	int playerNum;
	uint64_t serverTick;
	uint64_t serverTimestamp;
};

struct PingpongPacket
{
	char packetType = CS_PingpongInfo;
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
	char packetType = SC_PlayerInfo;
	PlayerInfoPacket playerInfos[4];
	int serverTick;
};


struct PlayerKeyPacket {
	char packetType;
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
	char packetType = SC_ItemInfo;
	ItemInfoPacket itemInfos[10];
};

struct MissileInfoPacket {
	bool active = false;
	XMFLOAT3 position;
};

struct MissileInfoBundlePacket
{
	char packetType = SC_MissileInfo;
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

inline void ConvertFloat3toInt32(XMFLOAT3& position, int32_t& x, int32_t& y, int32_t& z, float scale)
{
	x = static_cast<int32_t>(position.x * scale);
	y = static_cast<int32_t>(position.y * scale);
	z = static_cast<int32_t>(position.z * scale);
}

inline XMFLOAT3 ConvertInt32tofloat3(int32_t& x, int32_t& y, int32_t& z, float scale)
{
	XMFLOAT3 newPosition;
	newPosition.x = static_cast<float>(x / scale);
	newPosition.y = static_cast<float>(y / scale);
	newPosition.z = static_cast<float>(z / scale);
	return newPosition;
}



