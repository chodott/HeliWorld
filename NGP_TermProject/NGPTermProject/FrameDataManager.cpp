#include "FrameDataManager.h"

float FrameDataManager::GetServerFrameData(ServerFrameData& prevData, ServerFrameData& nextData, const uint64_t& serverTime)
{
    std::lock_guard<std::mutex> lock(mtx);
    float value = 5.0f;
    bool bCanInterpolate = false;
    for (int i = 0; i + 1 < serverframeData_dq.size(); ++i) {
        if (serverframeData_dq[i].timestamp <= serverTime &&
            serverframeData_dq[i + 1].timestamp >= serverTime) {
            prevData = serverframeData_dq[i];
            nextData = serverframeData_dq[i + 1];
            bCanInterpolate = true;
            break;
        }
    }
    if (bCanInterpolate)
    {
        value = float(serverTime - prevData.timestamp) / float(nextData.timestamp - prevData.timestamp);
    }
    return value;
}

template<>
void FrameDataManager::CombinePacket<ItemInfoBundlePacket>(const ItemInfoBundlePacket& pkt, uint64_t cutTimeline)
{
    std::lock_guard<std::mutex> lock(mtx);
    memcpy(currentFrameData.itemInfos, pkt.itemInfos, sizeof(ItemInfoPacket) * 10);
    serverframeData_dq.push_back(currentFrameData);


    if (!CheckPrediction(currentFrameData.timestamp))
    {
        Resimulate(currentFrameData.timestamp);
    }

    while (!serverframeData_dq.empty() && serverframeData_dq.front().timestamp < cutTimeline)
    {
        serverframeData_dq.pop_front();
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


auto cmpTimestamp = [](const ClientFrameData& lhs, const uint64_t& value) {
    return lhs.timestamp < value;
};

bool FrameDataManager::CheckPrediction(uint64_t timestamp)
{
    auto target =  lower_bound(clientFrameData_dq.begin(), clientFrameData_dq.end(), timestamp, cmpTimestamp);
    
    XMFLOAT3& clientPosition = target->position;
    XMFLOAT3& serverPosition = currentFrameData.playerInfos[0].position;
    float distance = 0.0f;

    distance = sqrt(pow(2, (clientPosition.x - serverPosition.x)) + pow(2, (clientPosition.y - serverPosition.y)) + pow(2, (clientPosition.z - serverPosition.z)));
    return distance < 1.0f: true ? false;
}
