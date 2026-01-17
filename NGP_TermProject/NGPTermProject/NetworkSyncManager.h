#pragma once
#include <deque>
#include <numeric>
#include <chrono>

#include "IPacketListener.h"

#define MAX_DEQUE_LENGTH 10

class NetworkSyncManager : public IPacketListener
{
public:
	uint64_t GetTimestampMs();
	double GetEstimatedServerTime();
	double GetDelayedServerTick();
	double GetEstimatedServerTick();
	uint64_t GetEstimatedServerTickI();
	inline uint64_t GetUpdatedTick() { return updatedTick; }
	bool UpdateServerTick();
	void UpdateSyncData(const uint64_t clientSendTimestamp, const uint64_t serverSendTimestamp);

	void SetBase(uint64_t tick, uint64_t timestamp) { baseTick = tick; baseServerTimestamp = timestamp; }
	inline static float GetRttAvg() { return rttAvg; }

	deque<double> scOffset_dq;
	deque<double> rtt_dq;

private:
	static double offsetAvg;
	static double rttAvg;
	double delayTick;
	uint64_t baseTick;
	uint64_t baseServerTimestamp;

	uint64_t updatedTick;


	// IPacketListener을(를) 통해 상속됨
	virtual void OnReceivePacket(char packetType, const char* buffer) override;

};

