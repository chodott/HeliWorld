#pragma once
#include "SCPacket.h"
#include <concurrent_queue.h>

class SnapshotPacketBuffer
{
private:
	concurrency::concurrent_queue<TickSnapshotPacket>  snapshotPacketQueue;

public:
	void PushSnapshotPacket(const TickSnapshotPacket& snapshotPacket);
	bool TryGetSnapshotPacket(TickSnapshotPacket& snapshotPacket);
};

