#pragma once
#include <deque>
#include <numeric>
#include <chrono>

#include "CSPacket.h"

#define DEFAULT_DELAY_MS 80
#define MAX_SYNC_SAMPLE_SIZE 10

class NetworkSyncManager
{
public:
	uint64_t GetTimestampMs();
	uint64_t GetEstimatedServerTimeMs();
	uint64_t GetDelayedServerTimeMs();
	void UpdateSyncData(const uint64_t clientSendTimestamp, const uint64_t serverSendTimestamp);

	inline static float GetRttAvg() { return rttAvg; }

	deque<float> scOffset_dq;
	deque<float> rtt_dq;

private:
	float CalculateAvg(deque<float>& target_dq);

	static float offsetAvg;
	static float rttAvg;
	int delay;

};

