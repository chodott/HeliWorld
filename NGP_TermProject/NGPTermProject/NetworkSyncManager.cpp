#include "NetworkSyncManager.h"

float NetworkSyncManager::offsetAvg = 0.f;
float NetworkSyncManager::rttAvg = 0.f;

template<class T> 
void KeepDequeSize(deque<T>& dq)
{
	if (dq.size() <= MAX_DEQUE_LENGTH) 
	{
		return;
	}
	dq.erase(dq.begin(), dq.begin() + (dq.size() - MAX_DEQUE_LENGTH));
}

template<class T>
T GetDequeAvg(const deque<T>& dq)
{
	if(dq.empty())
	{
		return 0;
	}
	T avg = std::accumulate(dq.begin(), dq.end(), 0.f) / dq.size();
	return avg;
}

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

void NetworkSyncManager::UpdateSyncData(const uint64_t clientSendTimestamp, 
										const uint64_t serverSendTimestamp)
{
	uint64_t rtt = GetTimestampMs() - clientSendTimestamp;
	float offset = serverSendTimestamp - (clientSendTimestamp + rtt * 0.5f);
	scOffset_dq.push_back(offset);
	rtt_dq.push_back(rtt);

	//Calculate Offset Avg
	KeepDequeSize(scOffset_dq);
	offsetAvg = GetDequeAvg(scOffset_dq);
	//Calculate Rtt Avg
	KeepDequeSize(rtt_dq);
	rttAvg = GetDequeAvg(rtt_dq);
	delay = (int)(rttAvg * 0.5f) + DEFAULT_DELAY_MS;
}