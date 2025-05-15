#pragma once
#include "CSPacket.h"
#include <deque>
struct FrameData {
    uint64_t timestamp;
    PlayerInfoPacket playerInfos[4];
    ItemInfoPacket itemInfos[10];
    MissileInfoPacket missileInfos[32];
};

class FrameDataManager
{
public:
    template<typename PacketType>
    void CombinePacket(const PacketType& packet, uint64_t cutTimeline);

    float GetFrameData(FrameData*& prevData, FrameData*& nextData, const uint64_t& serverTime);

private:
    std::deque<FrameData> frameData_dq;
    FrameData currentFrameData;
};


