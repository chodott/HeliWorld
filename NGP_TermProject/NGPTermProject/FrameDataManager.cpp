#include "FrameDataManager.h"
#include "Player.h"

void FrameDataManager::AddClientFrameData(const ClientFrameData& frameData)
{
	std::lock_guard<std::mutex> lock(frameDataLock);
	if (clientFrameData_dq.empty() == false)
	{
		const uint64_t prevTick = clientFrameData_dq.back().estimatedServerTick;
		const uint64_t newTick = frameData.estimatedServerTick;
		for (uint64_t curTick = prevTick + 1; curTick < newTick; ++curTick)
		{
			ClientFrameData newFrameData = clientFrameData_dq.back();
			newFrameData.estimatedServerTick = curTick;
			clientFrameData_dq.emplace_back(newFrameData);
		}
	}
	clientFrameData_dq.emplace_back(frameData);

	while (!clientFrameData_dq.empty() && clientFrameData_dq.front().estimatedServerTick < dataCutTickLine)
	{
		clientFrameData_dq.pop_front();
	}
}

void FrameDataManager::AddServerFrameData(const TickSnapshotPacket& snapshotPacket)
{
	{
		std::lock_guard<std::mutex> lock(frameDataLock);
		serverFrameData_dq.emplace_back(
			snapshotPacket.serverTick,
			snapshotPacket.playerInfos,
			snapshotPacket.missileInfos,
			snapshotPacket.itemInfos
			);

		dataCutTickLine = snapshotPacket.serverTick > SNAPSHOT_LIFETIME_TICK ? snapshotPacket.serverTick - SNAPSHOT_LIFETIME_TICK : 0;
		while (!serverFrameData_dq.empty() && serverFrameData_dq.front().serverTick < dataCutTickLine)
		{
			serverFrameData_dq.pop_front();
		}
	}

	CheckPositionOutOfSync();
}

bool FrameDataManager::TryGetClientFrameData(const uint64_t targetTick, ClientFrameData& frameData)
{
	std::lock_guard<std::mutex> lock(frameDataLock);
	auto it = std::lower_bound(
		clientFrameData_dq.begin(),clientFrameData_dq.end(),
		targetTick,
		[](const ClientFrameData& lhs, const uint64_t& value) {return lhs.estimatedServerTick < value;}
	);

	if (it != clientFrameData_dq.end() && it->estimatedServerTick == targetTick) {
		frameData = *it;
		return true;
	}

	return false;
}

bool FrameDataManager::TryGetServerFrameData(const uint64_t targetTick, ServerFrameData& prevData, ServerFrameData& nextData)
{
	std::lock_guard<std::mutex> lock(frameDataLock);
	if (serverFrameData_dq.size() < 2)
	{
		cout << "Lack of Server Frame Data" << "\n";
		return false;
	}

	auto it = std::lower_bound(
		serverFrameData_dq.begin(), serverFrameData_dq.end(),
		targetTick,
		[](const ServerFrameData& lhs, const uint64_t& value) { return lhs.serverTick < value; }
	);

	if (it == serverFrameData_dq.begin() || it == serverFrameData_dq.end())
	{
		cout << "Lack of Server Frame Data" << "\n";
		return false;
	}

	nextData = *it;
	prevData = *(it - 1);
	return true;
}

pair<uint64_t, uint64_t> FrameDataManager::GetSimulateTickRange()
{
	std::lock_guard<std::mutex> lock(frameDataLock);

	if (clientFrameData_dq.empty())
	{
		return { targetTick, targetTick };
	}

	uint64_t startTick = targetTick;
	uint64_t endTick = clientFrameData_dq.back().estimatedServerTick;
	if (startTick > endTick)
	{
		return std::make_pair(endTick, endTick);
	}
	return std::make_pair(targetTick, endTick);
}

bool FrameDataManager::TryGetClientStartIndex(uint64_t startTick, size_t& outIndex)
{
	std::lock_guard<std::mutex> lock(frameDataLock);
	auto it = std::lower_bound(clientFrameData_dq.begin(), clientFrameData_dq.end(),
		startTick, 
		[](const ClientFrameData& lhs, const uint64_t& value) {return lhs.estimatedServerTick < value; }
	);

	if (it == clientFrameData_dq.end() || it->estimatedServerTick != startTick)
	{
		return false;
	}

	outIndex = size_t(std::distance(clientFrameData_dq.begin(), it));                      
	return true;
}


void FrameDataManager::ReceiveMissileEvent(const LocalMissileEventPacket& pkt)
{
	MissileEventQueue.push(pkt);
}

bool FrameDataManager::ReadClientFrame(size_t index, ClientFrameData& outFrameData)
{
	std::lock_guard<std::mutex> lock(frameDataLock);
	if (index >= clientFrameData_dq.size())
	{
		return false;
	}
	outFrameData = clientFrameData_dq[index];
	return true;
}

bool FrameDataManager::WriteClientSimulateResult(size_t index, const XMFLOAT3& pos, const XMFLOAT3& rot)
{
	std::lock_guard<std::mutex> lock(frameDataLock);
	if (index >= clientFrameData_dq.size()) return false;
	clientFrameData_dq[index].position = pos;
	clientFrameData_dq[index].rotation = rot;
	return true;
}

void FrameDataManager::CheckPositionOutOfSync()
{
	ServerFrameData serverSnapData;
	uint64_t serverTick = 0;

	ClientFrameData prevClientFrameData{};
	ClientFrameData nextClientFrameData{};

	XMFLOAT3 rollbackBasePos{};
	XMFLOAT3 rollbackBaseRot{};

	{
		std::lock_guard<std::mutex> dataLock(frameDataLock);
		if (serverFrameData_dq.size() < 2 || clientFrameData_dq.size() < 2)
		{
			return;
		}

		serverSnapData = serverFrameData_dq.back();
		serverTick = serverSnapData.serverTick;
		auto clientNextIter = lower_bound(
			clientFrameData_dq.begin(), clientFrameData_dq.end(),
			serverTick, 
			[](const ClientFrameData& lhs, const uint64_t& value) {return lhs.estimatedServerTick < value; }
		);

		if (clientNextIter == clientFrameData_dq.begin())
		{
			return;
		}
		if (clientNextIter == clientFrameData_dq.end())
		{
			--clientNextIter;
		}
		auto clientPrevIter = clientNextIter - 1;

		prevClientFrameData = *clientPrevIter;
		nextClientFrameData = *clientNextIter;

		targetTick = serverTick;
		const int index = serverFrameData_dq.size() - 2;
		rollbackBasePos = serverFrameData_dq[index].playerInfos[playerNum].position;
		rollbackBaseRot = serverFrameData_dq[index].playerInfos[playerNum].rotation;
	}

	float lerpAlpha = 0.0f;
	const uint64_t serverTickGap = nextClientFrameData.estimatedServerTick - prevClientFrameData.estimatedServerTick;
	if (serverTickGap == 0)
	{
		return;
	}

	lerpAlpha = float(serverTick - prevClientFrameData.estimatedServerTick) / (float)serverTickGap;
	lerpAlpha = Clamp(lerpAlpha, 0.0f, 1.0f);

	XMFLOAT3 clientPosition = LerpFloat3(prevClientFrameData.position, nextClientFrameData.position, lerpAlpha);
	XMFLOAT3 serverPosition = serverSnapData.playerInfos[playerNum].position;

	const float dx = clientPosition.x - serverPosition.x;
	const float dy = clientPosition.y - serverPosition.y;
	const float dz = clientPosition.z - serverPosition.z;
	const float distSq = dx * dx + dy * dy + dz * dz;

	float interpDelaySec = (NetworkSyncManager::GetRttAvg() * 0.5f + Protocol::kDefaultDelayMs) * Protocol::kMsToSec;
	const float maxDistance = Protocol::kSpeedPlayerPerSec * Protocol::kDistanceMargin * interpDelaySec;
	const float maxDistSq = maxDistance * maxDistance;
	const bool bOverMaxDistance = (distSq >= maxDistSq);

	XMFLOAT3 diff = XMFLOAT3(serverPosition.x - clientPosition.x,
													serverPosition.y - clientPosition.y,
													serverPosition.z - clientPosition.z);

	{
		lock_guard<std::mutex> resimLock(resimulateLock);
		if (bOverMaxDistance)
		{
			needResimulate = true;
			targetTick = serverTick;
			rollbackPosition = rollbackBasePos;
			rollbackRotation = rollbackBaseRot;

		}
		else
		{
			diffVector = diff;
		}
	}
}

bool FrameDataManager::IsNeedResimulation()
{
	lock_guard<std::mutex> lock(resimulateLock);
	return needResimulate;
}

void FrameDataManager::StepCorrection(const float alpha)
{
	lock_guard<std::mutex> lock(resimulateLock);
	XMVECTOR fullError = XMLoadFloat3(&diffVector);

	XMVECTOR stepError = XMVectorScale(fullError, alpha);

	XMVECTOR updatedError = XMVectorSubtract(fullError, stepError);
	XMStoreFloat3(&diffVector, updatedError);
}
