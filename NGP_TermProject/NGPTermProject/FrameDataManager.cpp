#include "FrameDataManager.h"

float FrameDataManager::GetFrameData(FrameData*& prevData, FrameData*& nextData, const uint64_t& serverTime)
{
    float value = 5.0f;
    bool bCanInterpolate = false;
    for (int i = 0; i + 1 < frameData_dq.size(); ++i) {
        if (frameData_dq[i].timestamp <= serverTime &&
            frameData_dq[i + 1].timestamp >= serverTime) {
            prevData = &frameData_dq[i];
            nextData = &frameData_dq[i + 1];
            bCanInterpolate = true;
            break;
        }
    }
    if (bCanInterpolate)
    {
        value = float(serverTime - prevData->timestamp) / float(nextData->timestamp - prevData->timestamp);

    }
    return value;
}

template<>
void FrameDataManager::CombinePacket<ItemInfoBundlePacket>(const ItemInfoBundlePacket& pkt, uint64_t cutTimeline)
{
    memcpy(currentFrameData.itemInfos, pkt.itemInfos, sizeof(ItemInfoPacket) * 10);
    frameData_dq.push_back(currentFrameData);


    while (!frameData_dq.empty() && frameData_dq.front().timestamp < cutTimeline)
    {
        frameData_dq.pop_front();
    }
}

template<>
void FrameDataManager::CombinePacket<MissileInfoBundlePacket>(const MissileInfoBundlePacket& pkt, uint64_t cutTimeline)
{
    memcpy(currentFrameData.missileInfos, pkt.missileInfos, sizeof(MissileInfoPacket) * 32);
}

template<>
void FrameDataManager::CombinePacket<PlayerInfoBundlePacket>(const PlayerInfoBundlePacket& pkt, uint64_t cutTimeline)
{
    memcpy(currentFrameData.playerInfos, pkt.playerInfos, sizeof(PlayerInfoPacket) * 4);
    currentFrameData.timestamp = pkt.timestamp;
}
