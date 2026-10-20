#pragma once
#include "SCPacket.h"
#include "ProtocolConstants.h"
#include "TickRingBuffer.h"


#include <mutex>

class ClientInputBuffer
{
private:
	TickRingBuffer<PlayerInputPacket, Protocol::kPendingInputCapacity> pendingInputs;
	TickRingBuffer<PlayerInputPacket, Protocol::kInputHistoryCapacity> inputHistory;
	// 입력 허용 범위의 기준인 마지막 완료 틱. 새 세션에서도 서버 시간은 유지한다.
	uint64_t lastCompletedTick = 0;

	mutex inputBufferLock;
	bool resetPending = false;

public:
	void Clear();
	bool ConsumeReset();
	void CompleteTick(uint64_t tick);
	void PushInputData(const PlayerInputPacket& InputPacket);
	uint64_t GetResimulateStartTick(const uint64_t curTick);
	bool TryGetInputPacket(const uint64_t targetTick, PlayerInputPacket& outInputPacket);
};

