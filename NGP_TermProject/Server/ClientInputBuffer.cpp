#include "ClientInputBuffer.h"


void ClientInputBuffer::PushInputData(const PlayerInputPacket& inputPacket)
{
	std::lock_guard<std::mutex> lock(inputBufferLock);
	inputBuffer.push(inputPacket);
}

uint64_t ClientInputBuffer::GetResimulateStartTick(const uint64_t curTick)
{
	std::lock_guard<std::mutex> lock(inputBufferLock);
	uint64_t earliestTick = curTick;

	while (inputBuffer.empty() == false)
	{
		const uint64_t tick = inputBuffer.front().estimatedTick;
		if (tick > curTick)
		{
			break;
		}

		inputLogMap[tick] = inputBuffer.front();
		inputBuffer.pop();
		earliestTick = min(earliestTick, tick);
	}

	return earliestTick;
}

bool ClientInputBuffer::TryGetInputPacket(const uint64_t targetTick, PlayerInputPacket& outInputPacket)
{
	auto it = inputLogMap.find(targetTick);
	if (it != inputLogMap.end())
	{
		outInputPacket = it->second;
		return true;
	}
	return false;
}
