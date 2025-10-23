#pragma once
#include "CSPacket.h"
#include "FrameData.h"
#include "NetworkSyncManager.h"
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

    pair<std::deque<ClientFrameData>::iterator, std::deque<ClientFrameData>::iterator> GetSimulateRange();
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

    uint64_t targetTick;
    int serverTick;
    int playerNum = 0;
    bool needResimulate = false;

};


