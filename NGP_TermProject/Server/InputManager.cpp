#include "InputManager.h"

void InputManager::PushInputData(const int playerNum, const PlayerInputPacket& inputPacket)
{
	clientInputBuffers[playerNum].PushInputData(inputPacket);
}

uint64_t InputManager::GetResimulateStartTick(const uint64_t curTick)
{
	uint64_t earliestTick = curTick;
	for (int i = 0; i < Protocol::kMaxPlayerCount; ++i)
	{
		uint64_t tick = clientInputBuffers[i].GetResimulateStartTick(curTick);
		earliestTick = min(earliestTick, tick);
	}
	return earliestTick;
}

bool InputManager::TryGetInputPacket(const int playerNum, const uint64_t tick, PlayerInputPacket& outPacket)
{
	return clientInputBuffers[playerNum].TryGetInputPacket(tick, outPacket);
}