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

    uint64_t cutLine = frameData.serverTick > SNAPSHOT_LIFETIME_TICK ? frameData.serverTick - SNAPSHOT_LIFETIME_TICK : 0;

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

ClientFrameData* FrameDataManager::GetClientFrameData(uint64_t targetTick)
{
    auto it = std::lower_bound(
        clientFrameData_dq.begin(),
        clientFrameData_dq.end(),
        targetTick,
        cmpClientTick
    );

    if (it != clientFrameData_dq.end() && it->estimatedServerTick == targetTick) {
        return &(*it);
    }

    return nullptr;
}

bool FrameDataManager::GetCorrectionPos(XMFLOAT3& correctionPos)
{
    return (PosCorrectionDataQueue.try_pop(correctionPos));
}


bool FrameDataManager::GetServerFrameData(ServerFrameData& prevData, ServerFrameData& nextData, const uint64_t tick)
{
    std::lock_guard<std::mutex> lock(frameDataLock);
    bool bCanInterpolate = false;
    for (int i = 0; i + 1 < serverFrameData_dq.size(); ++i) {
        if (serverFrameData_dq[i].serverTick <= tick &&
            serverFrameData_dq[i + 1].serverTick > tick) {
            prevData = serverFrameData_dq[i];
            nextData = serverFrameData_dq[i + 1];
            return true;
        }
    }
    return false;
}

pair<uint64_t, uint64_t> FrameDataManager::GetSimulateTickRange()
{
    uint64_t startTick = targetTick;
    uint64_t endTick = clientFrameData_dq.back().estimatedServerTick;


    if (startTick > endTick)
    {
        return std::make_pair(endTick, endTick);
    }

   return std::make_pair(targetTick, endTick);
}

void FrameDataManager::ReceiveMissileEvent(const LocalMissileEventPacket& pkt)
{
    MissileEventQueue.push(pkt);
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

    float interpDelaySec = (NetworkSyncManager::GetRttAvg() * 0.5f + 50.0f) * 0.001f;
    float maxDistance = 150 * interpDelaySec;
    

    bool bOverMaxDistance = (distance >= maxDistance);

    lock_guard<std::mutex> lock(resimulateLock);

   if(bOverMaxDistance)
   {
        needResimulate = true;
        targetTick = serverTick;
        int index = serverFrameData_dq.size() - 2;
        basePosition = serverFrameData_dq[index].playerInfos[playerNum].position;
        baseRotation = serverFrameData_dq[index].playerInfos[playerNum].rotation;
   }
   else
   {
       PosCorrectionDataQueue.push(XMFLOAT3(serverPosition.x - clientPosition.x,
                                                                            serverPosition.y - clientPosition.y, 
                                                                        serverPosition.z - clientPosition.z));
   }
}

bool FrameDataManager::IsNeedResimulation()
{
    lock_guard<std::mutex> lock(resimulateLock);
    return needResimulate;
}
