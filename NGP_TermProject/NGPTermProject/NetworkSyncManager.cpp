#include "NetworkSyncManager.h"

float NetworkSyncManager::offsetAvg = 0.f;
float NetworkSyncManager::rttAvg = 0.f;

uint64_t NetworkSyncManager::GetEstimatedServerTimeMs(const uint64_t& fCurTimestampMs)
{
	return fCurTimestampMs + offsetAvg;
}

uint64_t NetworkSyncManager::GetDelayedServerTimeMs(const uint64_t& fCurTimestampMs)
{
	return GetEstimatedServerTimeMs(fCurTimestampMs) - delay;
}

void NetworkSyncManager::UpdateData(const PingpongPacket& ppPacket, const uint64_t& fCurTimestampMs)
{

	uint64_t rtt = fCurTimestampMs - ppPacket.clientTimeStamp;
	float offset = (float)ppPacket.serverSendTimeStamp - (float)(ppPacket.clientTimeStamp + (float)rtt / 2);
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