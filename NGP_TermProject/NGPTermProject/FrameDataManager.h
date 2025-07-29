#pragma once
#include "CSPacket.h"
#include "FrameData.h"
#include "NetworkSyncManager.h"
#include <mutex>

#define FRAMEDATA_DEADLINE_MS 5000



class FrameDataManager
{
public:
    template<typename PacketType>
    inline void SetPlayerNum(int n) 
    {
        playerNum = n;
    }
    inline void AddClientFrameData(const ClientFrameData& frameData)
    {
        clientFrameData_dq.emplace_back(frameData);
    }
    void AddServerFrameData(const ServerFrameData& frameData, uint64_t cutTimeline);

    float GetServerFrameData(ServerFrameData& prevData, ServerFrameData& nextData, const uint64_t& serverTime);
    pair<std::deque<ClientFrameData>::iterator, std::deque<ClientFrameData>::iterator> GetSimulateRange();
    bool IsPositionOutOfSync(const uint64_t& timestamp);

    void RequestResimulation(const uint64_t& timestamp);
    bool CheckResimulateRequest();

    XMFLOAT3 position;
    XMFLOAT3 rotation;

private:
    std::mutex mtx;
    std::mutex resimulateLock;
    std::deque<ServerFrameData> serverframeData_dq;
    std::deque<ClientFrameData> clientFrameData_dq;

    uint64_t targetTimestamp;
    int playerNum = 0;
    bool bNeedResimulate = false;

};


