
#pragma once

const char SC_PlayerInfo = 0;


#pragma pack(1)
struct PlayerInfoPacket {
    char PacketType;
    char PlayerNumber;
    float pitch, yaw, roll;
    float xDelta, yDelta, zDelta;
}
#pragma

