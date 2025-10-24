#include "FrameDataManager.h"

auto cmpClientTick = [](const ClientFrameData& lhs, const uint64_t& value) {
    return lhs.estimatedServerTick < value;
};
auto cmpServerTick = [](const ServerFrameData& lhs, const uint64_t& value) {
    return lhs.serverTick < value;
};

void FrameDataManager::AddServerFrameData(const ServerFrameData& frameData)
{
    std::lock_guard<std::mutex> lock(frameDataLock);
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

    CheckPositionOutOfSync();
}

float FrameDataManager::GetServerFrameData(ServerFrameData& prevData, ServerFrameData& nextData, const uint64_t tick)
{
    std::lock_guard<std::mutex> lock(frameDataLock);
    bool bCanInterpolate = false;
    float value = 1.0f;
    for (int i = 0; i + 1 < serverFrameData_dq.size(); ++i) {
        if (serverFrameData_dq[i].serverTick < tick &&
            serverFrameData_dq[i + 1].serverTick >= tick) {
            prevData = serverFrameData_dq[i];
            nextData = serverFrameData_dq[i + 1];
            bCanInterpolate = true;
            break;
        }
    }

    return value;
}

pair<std::deque<ClientFrameData>::iterator, std::deque<ClientFrameData>::iterator> FrameDataManager::GetSimulateRange()
{
   auto target = lower_bound(clientFrameData_dq.begin(), clientFrameData_dq.end(), targetTick, cmpClientTick);
   return make_pair(target, clientFrameData_dq.end());
}

void FrameDataManager::CheckPositionOutOfSync()
{
    auto serverSnapData = serverFrameData_dq.back();
    uint64_t serverTick = serverSnapData.serverTick;
    auto nextFrameData = lower_bound(clientFrameData_dq.begin(), clientFrameData_dq.end(),
        serverTick, cmpClientTick);

    if (nextFrameData == clientFrameData_dq.begin() || clientFrameData_dq.size() <= 1)
    {
        return;
    }
    else if (nextFrameData == clientFrameData_dq.end())
    {
        nextFrameData--;
    }
    auto prevFrameData = nextFrameData - 1;

    float t = (serverTick - prevFrameData->estimatedServerTick)/ 
                (nextFrameData->estimatedServerTick - prevFrameData->estimatedServerTick);
    XMFLOAT3 clientPosition = LerpFloat3(prevFrameData->position, nextFrameData->position, t);
    XMFLOAT3 serverPosition = serverSnapData.playerInfos[playerNum].position;
    clientPosition = prevFrameData->position;

    float distance = 0.0f;
    distance = sqrt(pow((clientPosition.x - serverPosition.x),2)
                    + pow((clientPosition.y - serverPosition.y),2)
                    + pow((clientPosition.z - serverPosition.z),2));

    /*cout << "client:" << clientPosition.x << ", "<<clientPosition.y << "," << clientPosition.z << endl;
    cout << "server:" << serverPosition.x << ", "<<serverPosition.y << "," << serverPosition.z << endl;*/


    float interpDelaySec = (NetworkSyncManager::GetRttAvg() * 0.5f + 20.0f) * 0.001f;
    float maxDistance = 100 * interpDelaySec;

    bool bOverMaxDistance = (distance >= maxDistance);
   // cout << distance << ", " << maxDistance << "\n";

   if(bOverMaxDistance)
   {
        lock_guard<std::mutex> lock(resimulateLock);
        needResimulate = true;
        targetTick = serverTick;
   }
}

bool FrameDataManager::IsNeedResimulation()
{
    lock_guard<std::mutex> lock(resimulateLock);
    return needResimulate;
}
