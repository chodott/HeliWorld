#pragma once
#include "CSPacket.h"
#include "FrameData.h"
#include "NetworkSyncManager.h"
#include <concurrent_queue.h>
#include <mutex>

#define SNAPSHOT_LIFETIME_TICK 50


class FrameDataManager
{
  public:
    inline void SetPlayerNum(int n) 
    {
        playerNum = n;
    }
    void AddClientFrameData(const ClientFrameData& frameData);
    void AddServerFrameData(const TickSnapshotPacket& frameData);

    pair<uint64_t, uint64_t> GetSimulateTickRange();
    bool TryGetClientStartIndex(uint64_t startTick, size_t& outIndex);
    inline XMFLOAT3 GetDiffVector() { return diffVector; }
    bool TryGetClientFrameData(const uint64_t targetTick, ClientFrameData& frameData);
    bool TryGetServerFrameData(const uint64_t serverTime, ServerFrameData& prevData, ServerFrameData& nextData);
    void CheckPositionOutOfSync();
    void ReceiveMissileEvent(const LocalMissileEventPacket& pkt);
    bool ReadClientFrame(size_t index, ClientFrameData& outFrameData);
    bool WriteClientSimulateResult(size_t index, const XMFLOAT3& pos, const XMFLOAT3& rot);
    bool TryGetMissileEvent(LocalMissileEventPacket& event)
    {
        if(MissileEventQueue.try_pop(event) == true)
        {
            return true;
        }
        return false;
    }
    bool IsNeedResimulation();
    void StepCorrection(const float alpha);

      // 롤백 재시뮬레이션이 완료되었을 때 플래그를 해제한다.
      inline void FinishResimulation()
      {
          std::lock_guard<std::mutex> lock(resimulateLock);
          needResimulate = false;
      }

    XMFLOAT3 rollbackPosition;
    XMFLOAT3 rollbackRotation;

private:
    std::mutex resimulateLock;
    std::mutex frameDataLock;
    std::deque<ServerFrameData> serverFrameData_dq;
    std::deque<ClientFrameData> clientFrameData_dq;
    Concurrency::concurrent_queue<LocalMissileEventPacket> MissileEventQueue;
    
    XMFLOAT3 diffVector;
    uint64_t targetTick;
    uint64_t dataCutTickLine;
    int serverTick;
    int playerNum = 0;
    bool needResimulate = false;

};


