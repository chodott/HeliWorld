#include "ClientInputBuffer.h"


void ClientInputBuffer::PushInputData(const int index, const uint64_t currentTick, const PlayerKeyPacket& keyPacket)
{
	if (inputBuffers[index].empty() == false)
	{	//Remove past Input
		int tickDiff = currentTick - keyPacket.estimatedTick;
		if (tickDiff > MAX_REWIND_TICKS)
		{
			return;
		}
	}

	inputBuffers[index].push(keyPacket);
}


uint64_t ClientInputBuffer::GetResimulateStartTick(const uint64_t curTick)
{
	uint64_t startTick = curTick;
	for (int index = 0; index < Protocol::kMaxPlayerCount; ++index)
	{
		while (!inputBuffers->empty())
		{
			uint64_t tick = inputBuffers[index].front().estimatedTick;
			if (tick > curTick)
			{
				break;
			}

			inputLogMaps[index][tick] = inputBuffers[index].front();
			inputBuffers[index].pop();
			startTick = min(startTick, tick);
		}
	}
	return startTick;
}

bool ClientInputBuffer::TryGetKeyPacket(int clientNum, uint64_t targetTick, PlayerKeyPacket& outKeyPacket)
{
	if (inputLogMaps[i].find(targetTick) != inputLogMaps[i].end())
	{
		outKeyPacket = inputLogMaps[clientNum][targetTick];
		return true;
	}
	else
	{
		return false;
	}
}
