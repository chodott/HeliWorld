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
};


struct PlayerKeyPacket {
	char packetType;
	unsigned char playerKeyInput;
	FPoint deltaMouse;
	int16_t requestMissileNum;
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
	bool active = false;
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

inline void ConvertFloat3toInt16(XMFLOAT3& position, int16_t& x, int16_t& y, int16_t& z, float scale)
{
	x = static_cast<int16_t>(position.x * scale);
	y = static_cast<int16_t>(position.y * scale);
	z = static_cast<int16_t>(position.z * scale);
}

inline XMFLOAT3 ConvertInt16tofloat3(int16_t& x, int16_t& y, int16_t& z, float scale)
{
	XMFLOAT3 newPosition;
	newPosition.x = static_cast<float>(x / scale);
	newPosition.y = static_cast<float>(y / scale);
	newPosition.z = static_cast<float>(z / scale);
	return newPosition;
}



