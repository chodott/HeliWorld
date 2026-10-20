#include "ClientInputBuffer.h"

void ClientInputBuffer::Clear()
{
	std::lock_guard<std::mutex> lock(inputBufferLock);
	pendingInputs.Clear();
	inputHistory.Clear();
	resetPending = true;
}

bool ClientInputBuffer::ConsumeReset()
{
	std::lock_guard<std::mutex> lock(inputBufferLock);
	const bool pending = resetPending;
	resetPending = false;
	return pending;
}

void ClientInputBuffer::PushInputData(const PlayerInputPacket& inputPacket)
{
	std::lock_guard<std::mutex> lock(inputBufferLock);
	if (Protocol::IsInputTickAllowed(inputPacket.estimatedTick, lastCompletedTick))
		pendingInputs.Store(inputPacket.estimatedTick, inputPacket);
}

uint64_t ClientInputBuffer::GetResimulateStartTick(const uint64_t curTick)
{
	std::lock_guard<std::mutex> lock(inputBufferLock);
	if (curTick == 0)
		return 0;
	// curTick은 이번에 실행할 틱 N이며, 범위 검증의 기준 T는 N-1이다.
	lastCompletedTick = curTick - 1;
	const uint64_t oldestTick = Protocol::GetOldestInputTick(lastCompletedTick);
	inputHistory.DiscardBefore(oldestTick);
	pendingInputs.DiscardBefore(oldestTick);
	uint64_t earliestTick = curTick;
	// 미래 입력은 대기 칸에 남긴다. 앞선 미래 입력이 과거 입력 처리를 막지 않는다.
	const uint64_t tickCount = curTick - oldestTick + 1;
	for (uint64_t offset = 0; offset < tickCount; ++offset)
	{
		const uint64_t tick = oldestTick + offset;
		if (const auto* input = pendingInputs.Find(tick))
		{
			inputHistory.Store(tick, *input);
			pendingInputs.Erase(tick);
			earliestTick = min(earliestTick, tick);
		}
	}

	return earliestTick;
}

bool ClientInputBuffer::TryGetInputPacket(const uint64_t targetTick, PlayerInputPacket& outInputPacket)
{
	std::lock_guard<std::mutex> lock(inputBufferLock);
	const auto* input = inputHistory.Find(targetTick);
	if (input)
	{
		outInputPacket = *input;
		return true;
	}
	return false;
}

void ClientInputBuffer::CompleteTick(uint64_t tick)
{
	std::lock_guard<std::mutex> lock(inputBufferLock);
	lastCompletedTick = tick;
	const uint64_t oldestTick = Protocol::GetOldestInputTick(lastCompletedTick);
	inputHistory.DiscardBefore(oldestTick);
	pendingInputs.DiscardBefore(oldestTick);
}
