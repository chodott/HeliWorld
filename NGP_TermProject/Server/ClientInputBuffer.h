#pragma once
#include "SCPacket.h"
#include "ProtocolConstants.h"


#include <queue>
#include <unordered_map>

#define MAX_REWIND_TICKS 3

class ClientInputBuffer
{
private:
	queue<PlayerKeyPacket> inputBuffers[Protocol::kMaxPlayerCount];
	unordered_map<uint64_t, PlayerKeyPacket> inputLogMaps[Protocol::kMaxPlayerCount];

public:
	ClientInputBuffer() {}
	~ClientInputBuffer() {}
	void PushInputData(const int index, const uint64_t currentTick, const PlayerKeyPacket& keyPacket);
	uint64_t GetResimulateStartTick(const uint64_t curTick);
	bool TryGetKeyPacket(int clientNum, uint64_t targetTick, PlayerKeyPacket& outKeyPacket);
};

