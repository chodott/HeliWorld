#include "ClientInputBuffer.h"


void ClientInputBuffer::PushInputData(const int playerNum, const uint64_t currentTick, const PlayerInputPacket& inputPacket)
{
	queue<PlayerInputPacket>& inputBuffer = inputBuffers[playerNum];
	if (inputBuffer.empty() == false)
	{	
		int tickDiff = int(currentTick - inputPacket.estimatedTick);
		if (tickDiff > MAX_REWIND_TICKS)
		{
			return;
		}
	}
	inputBuffer.push(inputPacket);
}

uint64_t ClientInputBuffer::GetResimulateStartTick(const uint64_t curTick)
{
	uint64_t startTick = curTick;
	for (int i = 0; i < Protocol::kMaxPlayerCount; ++i)
	{
		queue<PlayerInputPacket>& inputBuffer = inputBuffers[i];
		while (inputBuffer.empty() == false)
		{
			const uint64_t tick = inputBuffer.front().estimatedTick;
			if (tick > curTick)
			{
				break;
			}

			inputLogMaps[i][tick] = inputBuffer.front();
			inputBuffer.pop();
			startTick = min(startTick, tick);
		}
	}
	return startTick;
}

bool ClientInputBuffer::TryGetInputPacket(int clientNum, uint64_t targetTick, PlayerInputPacket& outInputPacket)
{
	unordered_map<uint64_t, PlayerInputPacket>& inputLogMap = inputLogMaps[clientNum];
	auto it = inputLogMap.find(targetTick);
	if (it != inputLogMap.end())
	{
		outInputPacket = it->second;
		return true;
	}
	return false;
}
