#pragma once
struct ServerFrameData {
    uint64_t timestamp;
    PlayerInfoPacket playerInfos[4];
    ItemInfoPacket itemInfos[10];
    MissileInfoPacket missileInfos[32];
};

struct ClientFrameData
{
    uint64_t timestamp;
    XMFLOAT3 position;
    XMFLOAT3 rotation;
    unsigned char playerKeyInput;
    FPoint deltaMouse;
    float deltaTime;
};