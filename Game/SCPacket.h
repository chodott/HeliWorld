
#pragma once

const char SC_PlayerInfo = 0;
const char SC_PlayerStatus = 1;


#pragma pack(1)
struct PlayerInfoPacket {
    char PacketType;
    char PlayerNumber;
    float pitch, yaw, roll;
    float xDelta, yDelta, zDelta;
}

struct PlayerStatusPacket {
    char PacketType;
    char PlayerNumber;
    char ActivatedMissiles;
    int PlayerHP;
};

#pragma pack()