#pragma once
#include "SCPacket.h"
#include "ProtocolConstants.h"

#include <concurrent_queue.h>

class SnapshotPacketBuffer
{
private:
	concurrency::concurrent_queue<TickSnapshotPacket>  snapshotPacketQueue;
	HANDLE sendReadyEvent = nullptr;

public:
	SnapshotPacketBuffer();
	~SnapshotPacketBuffer();

	void PushSnapshotPacket(const TickSnapshotPacket& snapshotPacket);
	bool TryGetSnapshotPacket(TickSnapshotPacket& snapshotPacket);
	inline HANDLE GetSendEvent() const { return sendReadyEvent; }
};

