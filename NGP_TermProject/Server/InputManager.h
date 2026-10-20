#pragma once
#include "ClientInputBuffer.h"

#include <array>

class InputManager
{
private :

	array<ClientInputBuffer, Protocol::kMaxPlayerCount> clientInputBuffers;

public:
	void ClearPlayerInputs(int playerNum);
	bool ConsumeInputReset(int playerNum);
	void CompleteTick(uint64_t tick);
	void PushInputData(const int playerNum, const PlayerInputPacket& inputPacket);
	uint64_t GetResimulateStartTick(const uint64_t curTick);
	bool TryGetInputPacket(const int playerNum, const uint64_t tick, PlayerInputPacket& outPacket);
};

