#include "FrameDataManager.h"

void FrameDataManager::AddClientFrameData(const ClientFrameData& frameData)
{
	std::lock_guard<std::mutex> lock(frameDataLock);
	const uint64_t newTick = frameData.estimatedServerTick;
	if (hasClientFrames && newTick <= latestClientTick)
	{
		if (newTick == latestClientTick)
			clientFrameHistory.Store(newTick, frameData);
		return;
	}

	// 서버 수신과 독립적인 최신 로컬 예측 틱 N으로 보관 범위를 계산한다.
	const uint64_t oldestPredictionTick = newTick > Protocol::kClientPredictionHistoryTicks
		? newTick - Protocol::kClientPredictionHistoryTicks : 0;
	if (hasClientFrames)
	{
		const ClientFrameData previousFrame = *clientFrameHistory.Find(latestClientTick);
		const uint64_t nextTick = latestClientTick + 1;
		const uint64_t firstFillTick = nextTick < oldestPredictionTick ? oldestPredictionTick : nextTick;
		// 큰 틱 점프도 현재 보관 범위 안의 누락 틱만 채운다.
		for (uint64_t tick = firstFillTick; tick < newTick; ++tick)
		{
			ClientFrameData missingFrame = previousFrame;
			missingFrame.estimatedServerTick = tick;
			clientFrameHistory.Store(tick, missingFrame);
		}
	}
	clientFrameHistory.Store(newTick, frameData);
	latestClientTick = newTick;
	hasClientFrames = true;

	std::lock_guard<std::mutex> resimLock(resimulateLock);
	if (needResimulate && !clientFrameHistory.Find(targetTick))
	{
		needResimulate = false;
		diffVector = {};
	}
}

void FrameDataManager::AddServerFrameData(const TickSnapshotPacket& snapshotPacket)
{
	{
		std::lock_guard<std::mutex> lock(frameDataLock);
		// 수신 순서가 아닌 가장 큰 서버 틱 T가 보관 범위의 기준이다.
		const uint64_t oldestServerTick = latestServerTick > Protocol::kServerSnapshotHistoryTicks
			? latestServerTick - Protocol::kServerSnapshotHistoryTicks : 0;
		if (hasServerFrames && snapshotPacket.serverTick < oldestServerTick)
			return;
		if (!hasServerFrames || snapshotPacket.serverTick > latestServerTick)
		{
			latestServerTick = snapshotPacket.serverTick;
			hasServerFrames = true;
			const uint64_t newOldestTick = latestServerTick > Protocol::kServerSnapshotHistoryTicks
				? latestServerTick - Protocol::kServerSnapshotHistoryTicks : 0;
			// 틱이 건너뛰어도 예전 칸이 유효한 상태로 남지 않도록 만료시킨다.
			serverFrameHistory.DiscardBefore(newOldestTick);
		}
		serverFrameHistory.Store(snapshotPacket.serverTick, ServerFrameData(
			snapshotPacket.serverTick, snapshotPacket.playerInfos,
			snapshotPacket.missileInfos, snapshotPacket.itemInfos));
	}
	CheckPositionOutOfSync();
}

bool FrameDataManager::TryGetClientFrameData(const uint64_t tick, ClientFrameData& frameData)
{
	std::lock_guard<std::mutex> lock(frameDataLock);
	const auto* stored = clientFrameHistory.Find(tick);
	if (!stored)
		return false;
	frameData = *stored;
	return true;
}

bool FrameDataManager::TryGetServerFrameData(const uint64_t targetTick, ServerFrameData& prevData, ServerFrameData& nextData)
{
	std::lock_guard<std::mutex> lock(frameDataLock);
	if (!hasServerFrames || targetTick > latestServerTick)
		return false;
	const uint64_t oldestServerTick = latestServerTick > Protocol::kServerSnapshotHistoryTicks
		? latestServerTick - Protocol::kServerSnapshotHistoryTicks : 0;
	if (targetTick < oldestServerTick)
		return false;

	const ServerFrameData* previous = nullptr;
	const ServerFrameData* next = nullptr;
	// 최대 51칸만 확인하며 누락된 틱을 건너뛰어 보간 시점의 앞뒤 상태를 찾는다.
	const uint64_t tickCount = latestServerTick - oldestServerTick + 1;
	for (uint64_t offset = 0; offset < tickCount; ++offset)
	{
		const uint64_t tick = oldestServerTick + offset;
		const auto* stored = serverFrameHistory.Find(tick);
		if (!stored)
			continue;
		if (tick < targetTick)
			previous = stored;
		else
		{
			next = stored;
			break;
		}
	}
	if (!previous || !next)
		return false;
	prevData = *previous;
	nextData = *next;
	return true;
}

pair<uint64_t, uint64_t> FrameDataManager::GetSimulateTickRange()
{
	std::lock_guard<std::mutex> lock(frameDataLock);
	std::lock_guard<std::mutex> resimLock(resimulateLock);
	return { targetTick, hasClientFrames ? latestClientTick : targetTick };
}

void FrameDataManager::ReceiveMissileEvent(const LocalMissileEventPacket& pkt)
{
	MissileEventQueue.push(pkt);
}

bool FrameDataManager::WriteClientSimulateResult(uint64_t tick, const XMFLOAT3& pos, const XMFLOAT3& rot)
{
	std::lock_guard<std::mutex> lock(frameDataLock);
	const auto* stored = clientFrameHistory.Find(tick);
	if (!stored)
		return false;
	ClientFrameData updated = *stored;
	updated.position = pos;
	updated.rotation = rot;
	clientFrameHistory.Store(tick, updated);
	return true;
}

void FrameDataManager::CheckPositionOutOfSync()
{
	ServerFrameData serverSnapshot;
	ClientFrameData clientPrediction{};
	uint64_t serverTick = 0;
	{
		std::lock_guard<std::mutex> lock(frameDataLock);
		if (!hasServerFrames || !hasClientFrames)
			return;
		serverTick = latestServerTick;
		const auto* server = serverFrameHistory.Find(serverTick);
		const auto* client = clientFrameHistory.Find(serverTick);
		if (!server || !client)
			return;
		serverSnapshot = *server;
		clientPrediction = *client;
	}

	const XMFLOAT3 serverPosition = serverSnapshot.playerInfos[playerNum].position;
	const XMFLOAT3 clientPosition = clientPrediction.position;
	const XMFLOAT3 diff{ serverPosition.x - clientPosition.x,
		serverPosition.y - clientPosition.y, serverPosition.z - clientPosition.z };
	const float distSq = diff.x * diff.x + diff.y * diff.y + diff.z * diff.z;
	const float delaySeconds = (NetworkSyncManager::GetRttAvg() * 0.5f + Protocol::kDefaultDelayMs) * Protocol::kMsToSec;
	const float maxDistance = Protocol::kSpeedPlayerPerSec * Protocol::kDistanceMargin * delaySeconds;

	std::lock_guard<std::mutex> lock(frameDataLock);
	if (serverTick != latestServerTick || !clientFrameHistory.Find(serverTick))
		return;
	std::lock_guard<std::mutex> resimLock(resimulateLock);
	if (distSq >= maxDistance * maxDistance)
	{
		needResimulate = true;
		targetTick = serverTick;
		// 기준 틱과 위치를 같은 서버 스냅샷으로 맞춘 뒤 그 다음 입력부터 재실행한다.
		rollbackPosition = serverPosition;
		rollbackRotation = serverSnapshot.playerInfos[playerNum].rotation;
	}
	else
		diffVector = diff;
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

void FrameDataManager::OnReceivePacket(char packetType, const char* buffer)
{
	switch (packetType)
	{
	case PACKET::SnapshotInfo:
	{
		auto& pkt = *reinterpret_cast<const TickSnapshotPacket*>(buffer);
		AddServerFrameData(pkt);
		break;
	}

	case PACKET::LocalMissileEvent:
	{
		auto& pkt = *reinterpret_cast<const LocalMissileEventPacket*>(buffer);
		ReceiveMissileEvent(pkt);
		break;
	}
	}
}
