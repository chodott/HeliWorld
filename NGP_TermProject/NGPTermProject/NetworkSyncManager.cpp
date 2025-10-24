#include "NetworkSyncManager.h"

float NetworkSyncManager::offsetAvg = 0.f;
float NetworkSyncManager::rttAvg = 0.f;

constexpr double kDt = 1.0 / 60.0;
constexpr double kDtMs = kDt * 1000.0;

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
	return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>
		(steady_clock::now().time_since_epoch()).count();
}

uint64_t NetworkSyncManager::GetEstimatedServerTimeMs()
{
	return GetTimestampMs() + offsetAvg - rttAvg * 0.5f;
}

uint64_t NetworkSyncManager::GetDelayedServerTick()
{
	return GetEstimatedServerTick() - delayTick;
}

uint64_t NetworkSyncManager::GetEstimatedServerTick()
{
	uint64_t elapsed = GetEstimatedServerTimeMs() - baseServerTimestamp;
	uint64_t serverTick = baseTick + (uint64_t)round(elapsed / kDtMs);
	return serverTick;
}

bool NetworkSyncManager::UpdateServerTick()
{
	uint64_t newTick = GetEstimatedServerTick();
	if (updatedTick >= newTick)
	{
		return false;
	}

	updatedTick = newTick;
	return true;
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
	float delay = (int)(rttAvg * 0.5f) + DEFAULT_DELAY_MS;
	delayTick = delay / kDtMs;
}