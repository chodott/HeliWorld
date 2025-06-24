#include "FrameDataManager.h"

auto cmpTimestamp = [](const ClientFrameData& lhs, const uint64_t& value) {
    return lhs.timestamp < value;
};

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

pair<std::deque<ClientFrameData>::iterator, std::deque<ClientFrameData>::iterator> FrameDataManager::GetSimulateRange(uint64_t timestamp)
{
    auto target = lower_bound(clientFrameData_dq.begin(), clientFrameData_dq.end(), timestamp, cmpTimestamp);
    return make_pair(target, clientFrameData_dq.end());
}

template<>
void FrameDataManager::CombinePacket<ItemInfoBundlePacket>(const ItemInfoBundlePacket& pkt, uint64_t cutTimeline)
{
    std::lock_guard<std::mutex> lock(mtx);
    memcpy(currentFrameData.itemInfos, pkt.itemInfos, sizeof(ItemInfoPacket) * 10);
    serverframeData_dq.push_back(currentFrameData);

    cutTimeline -= FRAMEDATA_DEADLINE_MS;

    if (CheckPrediction(currentFrameData.timestamp))
    {
        RequestResimulation(currentFrameData.timestamp);
    }
    while (!serverframeData_dq.empty() && serverframeData_dq.front().timestamp < cutTimeline)
    {
        serverframeData_dq.pop_front();
    }

    while (!clientFrameData_dq.empty() && clientFrameData_dq.front().timestamp < cutTimeline)
    {
        clientFrameData_dq.pop_front();
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

bool FrameDataManager::CheckPrediction(const uint64_t& timestamp)
{
    auto nextFrameData =  lower_bound(clientFrameData_dq.begin(), clientFrameData_dq.end(), timestamp, cmpTimestamp);
    if (nextFrameData == clientFrameData_dq.begin()) return false;
    if (nextFrameData == clientFrameData_dq.end()) return false;

    auto prevFrameData = nextFrameData - 1;

    XMVECTOR& nextPosition = XMLoadFloat3(&nextFrameData->position);
    XMVECTOR& prevPosition = XMLoadFloat3(&prevFrameData->position);
    float t = (timestamp - prevFrameData->timestamp) / (nextFrameData->timestamp - prevFrameData->timestamp);

    XMVECTOR& curPosition = XMVectorLerp(prevPosition, nextPosition, t);
    XMFLOAT3 clientPosition;
    XMStoreFloat3(&clientPosition, curPosition);
;
    XMFLOAT3& serverPosition = currentFrameData.playerInfos[playerNum].position;

    float distance = 0.0f;
    distance = sqrt(pow((clientPosition.x - serverPosition.x),2) + 
                            pow((clientPosition.y - serverPosition.y),2) + 
                            pow((clientPosition.z - serverPosition.z),2));
    
    float maxDistance = NetworkSyncManager::GetRttAvg() / 2 * 100.0f / 1000.0f;
    maxDistance = 10.0f;

    bool bOverMaxDistance = distance >= maxDistance;

    position = serverPosition;
    rotation = currentFrameData.playerInfos[0].rotation;

    return bOverMaxDistance;
}

void FrameDataManager::RequestResimulation(const uint64_t& timestamp)
{
    std::lock_guard<std::mutex>lock(resimulateLock);

    bNeedResimulate = true;
    targetTimestamp = timestamp;
}

bool FrameDataManager::CheckResimulateRequest(uint64_t& timestamp)
{
    std::lock_guard<std::mutex>lock(resimulateLock);

    if (!bNeedResimulate) return false;
    bNeedResimulate = false;
    timestamp = targetTimestamp;
    return true;

}
