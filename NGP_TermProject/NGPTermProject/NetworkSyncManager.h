#pragma once
#include <deque>
#include <numeric>
#include "CSPacket.h"

#define DEFAULT_DELAY_MS 80

class NetworkSyncManager
{
public:
	void UpdateData(const PingpongPacket& ppPacket, const uint64_t& fCurTimestampMs);
	uint64_t GetEstimatedServerTimeMs(const uint64_t& fCurTimestampMs);
	uint64_t GetDelayedServerTimeMs(const uint64_t& fCurTimestampMs);

	deque<float> scOffset_dq;
	deque<float> rtt_dq;

private:
	float offsetAvg;
	float rttAvg;
	int delay;

};

