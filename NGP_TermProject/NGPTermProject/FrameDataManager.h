#pragma once
#include "CSPacket.h"
#include <deque>
#include <mutex>
struct FrameData {
    uint64_t timestamp;
    PlayerInfoPacket playerInfos[4];
    ItemInfoPacket itemInfos[10];
    MissileInfoPacket missileInfos[32];
};

struct InputData
{
    uint64_t timestamp;
    unsigned char playerKeyInput;
    FPoint deltaMouse;
    float deltaTime;
};

class FrameDataManager
{
public:
    template<typename PacketType>
    void CombinePacket(const PacketType& packet, uint64_t cutTimeline);
    inline void AddInputData(uint64_t timestamp, unsigned char keyInput, const FPoint& deltaMouse, float deltaTime)
    {
        inputData_dq.push_back({ timestamp, keyInput, deltaMouse, deltaTime });
    }
    float GetFrameData(FrameData& prevData, FrameData& nextData, const uint64_t& serverTime);

private:
    std::mutex mtx;
    std::deque<FrameData> frameData_dq;
    std::deque<InputData> inputData_dq;
    FrameData currentFrameData;
};


