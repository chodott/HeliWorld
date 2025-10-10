#pragma once
#include <deque>
#include <numeric>
#include <chrono>

#include "CSPacket.h"

#define DEFAULT_DELAY_MS 80
#define MAX_DEQUE_LENGTH 10

class NetworkSyncManager
{
public:
	uint64_t GetTimestampMs();
	uint64_t GetEstimatedServerTimeMs();
	uint64_t GetDelayedServerTimeMs();
	uint64_t GetEstimatedServerTick();
	void UpdateSyncData(const uint64_t clientSendTimestamp, const uint64_t serverSendTimestamp);
	void SetBaseTick(uint64_t tick) { baseTick = tick;}
	inline static float GetRttAvg() { return rttAvg; }
	int GetDelay() { return delay; }

	deque<float> scOffset_dq;
	deque<float> rtt_dq;

private:
	static float offsetAvg;
	static float rttAvg;
	int delay;
	uint64_t baseTick;

};

