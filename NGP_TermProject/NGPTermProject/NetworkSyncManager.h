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
	float GetEstimatedServerTimeMs();
	float GetDelayedServerTick();
	float GetEstimatedServerTick();
	inline uint64_t GetUpdatedTick() { return updatedTick; }
	bool UpdateServerTick();
	void UpdateSyncData(const uint64_t clientSendTimestamp, const uint64_t serverSendTimestamp);
	void SetBase(uint64_t tick, uint64_t timestamp) { baseTick = tick; baseServerTimestamp = timestamp; }
	inline static float GetRttAvg() { return rttAvg; }

	deque<float> scOffset_dq;
	deque<float> rtt_dq;

private:
	static float offsetAvg;
	static float rttAvg;
	float delayTick;
	uint64_t baseTick;
	uint64_t baseServerTimestamp;

	uint64_t updatedTick;

};

