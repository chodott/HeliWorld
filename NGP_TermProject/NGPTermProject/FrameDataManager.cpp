#include "FrameDataManager.h"

auto cmpClientTick = [](const ClientFrameData& lhs, const uint64_t& value) {
    return lhs.estimatedServerTick < value;
};
auto cmpServerTick = [](const ServerFrameData& lhs, const uint64_t& value) {
    return lhs.serverTick < value;
};

void FrameDataManager::AddServerFrameData(const ServerFrameData& frameData)
{
    std::lock_guard<std::mutex> lock(mtx);
    serverFrameData_dq.emplace_back(frameData);
    serverTick = frameData.serverTick;

    int cutLine = 0;

    while (!serverFrameData_dq.empty() && serverFrameData_dq.front().serverTick < cutLine)
    {
        serverFrameData_dq.pop_front();
    }

    while (!clientFrameData_dq.empty() && clientFrameData_dq.front().estimatedServerTick < cutLine)
    {
        clientFrameData_dq.pop_front();
    }

    bool isOutOfSync = IsPositionOutOfSync();
}

float FrameDataManager::GetServerFrameData(ServerFrameData& prevData, ServerFrameData& nextData, const uint64_t tick)
{
    std::lock_guard<std::mutex> lock(mtx);
    float value = 5.0f;
    bool bCanInterpolate = false;
    //for (int i = 0; i + 1 < serverFrameData_dq.size(); ++i) {
    //    if (serverFrameData_dq[i].timestamp <= serverTime &&
    //        serverFrameData_dq[i + 1].timestamp >= serverTime) {
    //        prevData = serverFrameData_dq[i];
    //        nextData = serverFrameData_dq[i + 1];
    //        bCanInterpolate = true;
    //        break;
    //    }
    //}
    //if (bCanInterpolate)
    //{
    //    value = float(serverTime - prevData.timestamp) / float(nextData.timestamp - prevData.timestamp);
    //}
    return value;
}

//pair<std::deque<ClientFrameData>::iterator, std::deque<ClientFrameData>::iterator> FrameDataManager::GetSimulateRange()
//{
//    auto target = lower_bound(clientFrameData_dq.begin(), clientFrameData_dq.end(), targetTimestamp, cmpClientTimestamp);
//    return make_pair(target, clientFrameData_dq.end());
//}

bool FrameDataManager::IsPositionOutOfSync()
{
    auto serverSnapData = serverFrameData_dq.back();
    targetTick = serverSnapData.serverTick;
    auto nextFrameData =  lower_bound(clientFrameData_dq.begin(), clientFrameData_dq.end(), 
                                        targetTick, cmpClientTick);
    if (nextFrameData == clientFrameData_dq.begin() || nextFrameData == clientFrameData_dq.end())
    {
        if (!clientFrameData_dq.empty())
        {
            cout << "Tick:" << targetTick << ", " << clientFrameData_dq.back().estimatedServerTick << "\n";
        }
        return false;
    } 
    auto prevFrameData = nextFrameData - 1;

    float t = (targetTick - prevFrameData->estimatedServerTick)/ 
                (nextFrameData->estimatedServerTick - prevFrameData->estimatedServerTick);
    XMFLOAT3 clientPosition = LerpFloat3(prevFrameData->position, nextFrameData->position, t);
    XMFLOAT3 serverPosition = serverSnapData.playerInfos[playerNum].position;

    float distance = 0.0f;
    distance = sqrt(pow((clientPosition.x - serverPosition.x),2)
                    + pow((clientPosition.y - serverPosition.y),2)
                    + pow((clientPosition.z - serverPosition.z),2));

    float interpDelaySec = (NetworkSyncManager::GetRttAvg() * 0.5f + 20.0f) * 0.001f;
    float maxDistance = 100 * interpDelaySec;

    bool bOverMaxDistance = (distance >= maxDistance);
    cout << distance << ", " << maxDistance << "\n";

    return bOverMaxDistance;
}

bool FrameDataManager::CheckResimulateRequest()
{
    return IsPositionOutOfSync();
}
