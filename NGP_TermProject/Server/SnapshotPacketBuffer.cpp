#include "SnapshotPacketBuffer.h"


SnapshotPacketBuffer::SnapshotPacketBuffer()
{
	sendReadyEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
}

SnapshotPacketBuffer::~SnapshotPacketBuffer()
{
	CloseHandle(sendReadyEvent);
}

void SnapshotPacketBuffer::PushSnapshotPacket(const TickSnapshotPacket& snapshotPacket)
{
	snapshotPacketQueue.push(snapshotPacket);
	SetEvent(sendReadyEvent);
}

bool SnapshotPacketBuffer::TryGetSnapshotPacket(TickSnapshotPacket& snapshotPacket)
{
	return snapshotPacketQueue.try_pop(snapshotPacket);
}
