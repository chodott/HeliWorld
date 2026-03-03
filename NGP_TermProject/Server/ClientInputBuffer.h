#pragma once
#include "SCPacket.h"
#include "ProtocolConstants.h"


#include <queue>
#include <unordered_map>
#include <mutex>

#define MAX_REWIND_TICKS 3

class ClientInputBuffer
{
private:
	queue<PlayerInputPacket> inputBuffer;
	unordered_map<uint64_t, PlayerInputPacket> inputLogMap;

	mutex inputBufferLock;

public:
	void PushInputData(const uint64_t currentTick, const PlayerInputPacket& InputPacket);
	uint64_t GetResimulateStartTick(const uint64_t curTick);
	bool TryGetInputPacket(const uint64_t targetTick, PlayerInputPacket& outInputPacket);
};

