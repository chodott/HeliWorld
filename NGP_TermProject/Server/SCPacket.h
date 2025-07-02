#pragma once
#pragma warning(disable : 26495)

#include "stdafx.h"

const char SC_PlayerInfo = 0;
const char SC_ItemInfo = 1;
const char SC_MissileInfo = 2;
const char CS_KeyInfo = 3;
const char CS_PingpongInfo = 4;

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
	XMFLOAT3 rotation;
	int16_t launchedMissileNum = -1;
	uint64_t timestamp;
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
	bool active = false;
	int32_t positionX;  
	int32_t positionY;
	int32_t positionZ; 
};

struct MissileInfoBundlePacket
{
	char packetType;
	MissileInfoPacket missileInfos[32];
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



