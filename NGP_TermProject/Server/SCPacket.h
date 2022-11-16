
#pragma once

const char SC_PlayerInfo = 0;
const char SC_PlayerStatus = 1;


#pragma pack(1)
struct PlayerInfoPacket {
	PlayerInfoPacket() : packetType(SC_PlayerInfo) {}


	char packetType;
	char playerNumber;
	float pitch, yaw, roll;
	float xDelta, yDelta, zDelta;
};

struct PlayerStatusPacket {
	char PacketType;
	char PlayerNumber;
	char ActivatedMissiles;
	int PlayerHP;
};

#pragma pack()