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

    float GetServerFrameData(ServerFrameData& prevData, ServerFrameData& nextData, const uint64_t serverTime);
    //pair<std::deque<ClientFrameData>::iterator, std::deque<ClientFrameData>::iterator> GetSimulateRange();
    bool IsPositionOutOfSync();

    bool CheckResimulateRequest();

    XMFLOAT3 basePosition;
    XMFLOAT3 baseRotation;

private:
    std::mutex mtx;
    std::deque<ServerFrameData> serverFrameData_dq;
    std::deque<ClientFrameData> clientFrameData_dq;

    uint64_t targetTimestamp;
    int serverTick;
    int playerNum = 0;

};


