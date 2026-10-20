#pragma once
#include "FrameData.h"
#include "NetworkSyncManager.h"
#include "IPacketListener.h"
#include "../Common/TickRingBuffer.h"

#include <concurrent_queue.h>
#include <mutex>



class FrameDataManager : public IPacketListener
{
  public:
    inline void SetPlayerNum(int n) 
    {
        playerNum = n;
    }
    void AddClientFrameData(const ClientFrameData& frameData);
    void AddServerFrameData(const TickSnapshotPacket& frameData);

    pair<uint64_t, uint64_t> GetSimulateTickRange();
    XMFLOAT3 GetDiffVector() { std::lock_guard<std::mutex> lock(resimulateLock); return diffVector; }
    bool TryGetClientFrameData(const uint64_t targetTick, ClientFrameData& frameData);
    bool TryGetServerFrameData(const uint64_t serverTime, ServerFrameData& prevData, ServerFrameData& nextData);
    void CheckPositionOutOfSync();
    void ReceiveMissileEvent(const LocalMissileEventPacket& pkt);
    bool WriteClientSimulateResult(uint64_t tick, const XMFLOAT3& pos, const XMFLOAT3& rot);
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
    TickRingBuffer<ServerFrameData, Protocol::kServerSnapshotHistoryCapacity> serverFrameHistory;
    TickRingBuffer<ClientFrameData, Protocol::kClientPredictionHistoryCapacity> clientFrameHistory;
    // 원격 보간 기록은 마지막으로 받은 가장 큰 서버 틱을 기준으로 유효 범위를 정한다.
    uint64_t latestServerTick = 0;
    bool hasServerFrames = false;
    // 로컬 예측 기록은 서버 수신과 독립적인 최신 예측 틱을 기준으로 만료시킨다.
    uint64_t latestClientTick = 0;
    bool hasClientFrames = false;
    Concurrency::concurrent_queue<LocalMissileEventPacket> MissileEventQueue;
    
	XMFLOAT3 diffVector{};
	// 서버가 요청한 재시뮬레이션 기준 틱. 로컬 예측 기록의 삭제 기준으로 사용하지 않는다.
	uint64_t targetTick = 0;
    int playerNum = 0;
    bool needResimulate = false;


    // IPacketListener을(를) 통해 상속됨
    virtual void OnReceivePacket(char packetType, const char* buffer) override;

};


