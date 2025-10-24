#pragma once
#include "CSPacket.h"
#include "FrameData.h"
#include "NetworkSyncManager.h"
#include <concurrent_queue.h>
#include <mutex>


class FrameDataManager
{
public:
    inline void SetPlayerNum(int n) 
    {
        playerNum = n;
    }
    inline void AddClientFrameData(const ClientFrameData& frameData)
    {
        clientFrameData_dq.emplace_back(frameData);
    }
    void AddServerFrameData(const ServerFrameData& frameData);

    pair<uint64_t, uint64_t> GetSimulateTickRange();
    ClientFrameData* GetClientFrameData(uint64_t targetTick);
    XMFLOAT3 GetCorrectionPos();
    float GetServerFrameData(ServerFrameData& prevData, ServerFrameData& nextData, const uint64_t serverTime);
    void CheckPositionOutOfSync();
    bool IsNeedResimulation();

    XMFLOAT3 basePosition;
    XMFLOAT3 baseRotation;

private:
    std::mutex resimulateLock;
    std::mutex frameDataLock;
    std::deque<ServerFrameData> serverFrameData_dq;
    std::deque<ClientFrameData> clientFrameData_dq;
    Concurrency::concurrent_queue<XMFLOAT3> PosCorrectionDataQueue;

    uint64_t targetTick;
    int serverTick;
    int playerNum = 0;
    bool needResimulate = false;

};


