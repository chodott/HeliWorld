#pragma once
struct ServerFrameData {
    uint64_t serverTick;
    PlayerInfoPacket playerInfos[4];
    ItemInfoPacket itemInfos[10];
    MissileInfoPacket missileInfos[32];
};

struct ClientFrameData
{
    uint64_t estimatedServerTick;
    XMFLOAT3 position;
    XMFLOAT3 rotation;
    unsigned char playerKeyInput;
    FPoint deltaMouse;
};

