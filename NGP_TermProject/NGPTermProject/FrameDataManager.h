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
    void ReceiveMissileEvent(const LocalMissileEventPacket& pkt);
    bool TryGetMissileEvent(LocalMissileEventPacket& event)
    {
        if(MissileEventQueue.try_pop(event) == true)
        {
            return true;
        }
        return false;
    }
    bool IsNeedResimulation();

      // 롤백 재시뮬레이션이 완료되었을 때 플래그를 해제한다.
      inline void FinishResimulation()
      {
          std::lock_guard<std::mutex> lock(resimulateLock);
          needResimulate = false;
      }

    XMFLOAT3 basePosition;
    XMFLOAT3 baseRotation;

private:
    std::mutex resimulateLock;
    std::mutex frameDataLock;
    std::deque<ServerFrameData> serverFrameData_dq;
    std::deque<ClientFrameData> clientFrameData_dq;
    Concurrency::concurrent_queue<XMFLOAT3> PosCorrectionDataQueue;
    Concurrency::concurrent_queue<LocalMissileEventPacket> MissileEventQueue;

    uint64_t targetTick;
    int serverTick;
    int playerNum = 0;
    bool needResimulate = false;

};


