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

float CalculateAvg(deque<float>& target_dq)
{
	while (target_dq.size() > MAX_SYNC_SAMPLE_SIZE)
	{
		target_dq.pop_front();
	}
	return std::accumulate(target_dq.begin(), target_dq.end(), 0.f) / target_dq.size();

}

void NetworkSyncManager::UpdateSyncData(const uint64_t clientSendTimestamp, const uint64_t serverSendTimestamp)
{
	uint64_t rtt = GetTimestampMs() - clientSendTimestamp;
	float offset = (float)serverSendTimestamp - (float)(clientSendTimestamp + (float)rtt / 2);
	scOffset_dq.push_back(offset);
	rtt_dq.push_back(rtt);

	offsetAvg = CalculateAvg(scOffset_dq);
	rttAvg = CalculateAvg(rtt_dq);

	delay = (int)(rttAvg * 0.5f) + DEFAULT_DELAY_MS;
}