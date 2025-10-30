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
    XMFLOAT3 missilesPosition[8];
    bool missilesActive[8];
};

struct MissileCorrectionData
{
    int slotIndex;
    XMFLOAT3 correctionPos;
    uint64_t targetTick;
    bool active;
};
