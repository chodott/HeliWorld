#pragma once
struct ServerFrameData {
    uint64_t serverTick;
    PlayerInfoPacket playerInfos[4];
    ItemInfoPacket itemInfos[10];
    MissileInfoPacket missileInfos[32];
    ServerFrameData() {}
    ServerFrameData(uint64_t tick, const PlayerInfoPacket(&players)[4], const MissileInfoPacket(&missiles)[32], const ItemInfoPacket(&items)[10])
    {
        serverTick = tick;
        memcpy(playerInfos, players, sizeof(playerInfos));
        memcpy(missileInfos, missiles, sizeof(missileInfos));
        memcpy(itemInfos, items, sizeof(itemInfos));
    }
};

struct ClientFrameData
{
    uint64_t estimatedServerTick;
    XMFLOAT3 position;
    XMFLOAT3 rotation;
    unsigned char playerKeyInput;
    FPoint deltaMouse;
};

