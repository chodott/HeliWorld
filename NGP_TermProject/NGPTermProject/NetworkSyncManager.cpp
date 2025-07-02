#include "NetworkSyncManager.h"

float NetworkSyncManager::offsetAvg = 0.f;
float NetworkSyncManager::rttAvg = 0.f;

uint64_t NetworkSyncManager::GetTimestampMs()
{
	using namespace std::chrono;
	return (uint32_t)std::chrono::duration_cast<std::chrono::milliseconds>
		(steady_clock::now().time_since_epoch()).count();
}

uint64_t NetworkSyncManager::GetEstimatedServerTimeMs()
{
	return GetTimestampMs() + offsetAvg;
}

uint64_t NetworkSyncManager::GetDelayedServerTimeMs()
{
	return GetEstimatedServerTimeMs() - delay;
}

void NetworkSyncManager::UpdateSyncData(const uint64_t clientSendTimestamp, const uint64_t serverSendTimestamp)
{
	uint64_t rtt = GetTimestampMs() - clientSendTimestamp;
	float offset = (float)serverSendTimestamp - (float)(clientSendTimestamp + (float)rtt / 2);
	scOffset_dq.push_back(offset);
	rtt_dq.push_back(rtt);

	//Calculate Offset Avg
	while (scOffset_dq.size() > 10)
	{
		scOffset_dq.pop_front();
	}
	offsetAvg = std::accumulate(scOffset_dq.begin(), scOffset_dq.end(), 0.f) / scOffset_dq.size();

	//Calculate Rtt Avg
	while (rtt_dq.size() > 10)
	{
		rtt_dq.pop_front();
	}
	rttAvg = std::accumulate(rtt_dq.begin(), rtt_dq.end(),0.f) / rtt_dq.size();
	delay = (int)(rttAvg * 0.5f) + DEFAULT_DELAY_MS;
}