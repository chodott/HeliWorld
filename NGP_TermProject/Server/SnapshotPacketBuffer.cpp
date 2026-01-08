#include "SnapshotPacketBuffer.h"

void SnapshotPacketBuffer::PushSnapshotPacket(const TickSnapshotPacket& snapshotPacket)
{
	snapshotPacketQueue.push(snapshotPacket);
}

bool SnapshotPacketBuffer::TryGetSnapshotPacket(TickSnapshotPacket& snapshotPacket)
{
	return snapshotPacketQueue.try_pop(snapshotPacket);
}
