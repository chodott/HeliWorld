
#pragma once

const char CL_PlayerInfo = 0;
const char CL_PlayerStatus = 1;


#pragma pack(1)
struct PlayerInfoPacket {
    char PacketType;
    char PlayerNumber;
    XMFLOAT3  RotationAxis;
    XMFLOAT3 Rotation;
};

struct PlayerStatusPacket {
    char PacketType;
    char PlayerNumber;
    char ActivatedMissiles;
    int PlayerHP;
};

#pragma pack()