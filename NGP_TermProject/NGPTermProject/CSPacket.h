#pragma once
#include"stdafx.h"
const char CL_PlayerInfo = 0;
const char CL_PlayerStatus = 1;


#pragma pack(1)
struct PlayerInfoPacket {
    char packetType;
    char playerNumber;
    XMFLOAT3  movement;
    XMFLOAT3 rotation;
};

struct PlayerStatusPacket {
    char PacketType;
    char PlayerNumber;
    char ActivatedMissiles;
    int PlayerHP;
};

struct ItemInfoPacket {
    char packetType;
    float x, y, z;
    bool active;
};

#pragma pack()
