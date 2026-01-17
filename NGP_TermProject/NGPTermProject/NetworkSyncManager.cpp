#include "NetworkSyncManager.h"
#include "ProtocolConstants.h"

double NetworkSyncManager::offsetAvg = 0.f;
double NetworkSyncManager::rttAvg = 0.f;


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
double GetDequeAvgDouble(const deque<T>& dq)
{
	if(dq.empty())
	{
		return 0.0;
	}
	const double sum = std::accumulate(dq.begin(), dq.end(), 0.0);
	return sum / (double)dq.size();
}

uint64_t NetworkSyncManager::GetTimestampMs()
{
	using namespace std::chrono;
	return (uint64_t)std::chrono::duration_cast<std::chrono::milliseconds>
		(steady_clock::now().time_since_epoch()).count();
}

double NetworkSyncManager::GetEstimatedServerTime()
{
	return (double)GetTimestampMs() + offsetAvg - rttAvg * 0.5;
}

double NetworkSyncManager::GetDelayedServerTick()
{
	return GetEstimatedServerTick() - delayTick;
}

double NetworkSyncManager::GetEstimatedServerTick()
{
	double elapsed = GetEstimatedServerTime() - (double)baseServerTimestamp;
	double serverTick = (double)baseTick + elapsed / Protocol::kFixedTickMs;
	return serverTick;
}

uint64_t NetworkSyncManager::GetEstimatedServerTickI()
{
	double tickF = GetEstimatedServerTick();
	return (uint64_t)floor(tickF);
}

bool NetworkSyncManager::UpdateServerTick()
{
	uint64_t newTick = GetEstimatedServerTickI();
	if (updatedTick >= newTick)
	{
		return true;
	}

	updatedTick = newTick;
	return false;
}

void NetworkSyncManager::UpdateSyncData(const uint64_t clientSendTimestamp, 
										const uint64_t serverSendTimestamp)
{
	double rtt = double(GetTimestampMs() - clientSendTimestamp);
	double offset = double(serverSendTimestamp) - (double(clientSendTimestamp) + rtt * 0.5);
	scOffset_dq.push_back(offset);
	rtt_dq.push_back(rtt);

	//Calculate Offset Avg
	KeepDequeSize(scOffset_dq);
	offsetAvg = GetDequeAvgDouble(scOffset_dq);
	//Calculate Rtt Avg
	KeepDequeSize(rtt_dq);
	rttAvg = GetDequeAvgDouble(rtt_dq);
	double delay = (rttAvg * 0.5) + Protocol::kDefaultDelayMs;
	delayTick = delay / Protocol::kFixedTickMs;
}

void NetworkSyncManager::OnReceivePacket(char packetType, const char* buffer)
{
	if (packetType == PACKET::PingpongInfo)
	{
		const PingpongPacket* pkt = reinterpret_cast<const PingpongPacket*>(buffer);
		UpdateSyncData(pkt->clientTimeStamp, pkt->serverSendTimeStamp);
	}
}
