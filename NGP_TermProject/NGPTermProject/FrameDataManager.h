#pragma once
#include "CSPacket.h"
#include "NetworkSyncManager.h"
#include <mutex>

#define FRAMEDATA_DEADLINE_MS 5000

struct ServerFrameData {
    uint64_t timestamp;
    PlayerInfoPacket playerInfos[4];
    ItemInfoPacket itemInfos[10];
    MissileInfoPacket missileInfos[32];
};

struct ClientFrameData
{
    uint64_t timestamp;
    XMFLOAT3 position;
    XMFLOAT3 rotation;
    unsigned char playerKeyInput;
    FPoint deltaMouse;
    float deltaTime;
};

class FrameDataManager
{
public:
    template<typename PacketType>
    void CombinePacket(const PacketType& packet, uint64_t cutTimeline = 0);
    inline void SetPlayerNum(int n) 
    {
        playerNum = n;
    }
    inline void AddClientFrameData(const ClientFrameData& frameData)
    {
        clientFrameData_dq.emplace_back(frameData);
    }
    inline void AddServerFrameData(const ServerFrameData& frameData)
    {
        serverframeData_dq.emplace_back(frameData);
    }
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

    ServerFrameData currentFrameData;
    uint64_t targetTimestamp;
    int playerNum = 0;
    bool bNeedResimulate = false;

};


