#pragma once

#include "SCPacket.h"
#include <array>
#include <cstring>

// TCP may split a packet across reads or combine several packets in one read.
class PacketReceiveBuffer
{
public:
	static constexpr int kCapacity = 512;

	char* GetWriteBuffer() { return buffer.data() + bufferedSize; }
	int GetFreeSize() const { return kCapacity - bufferedSize; }

	template<typename Handler>
	bool ProcessReceived(int receivedSize, Handler&& handlePacket)
	{
		if (receivedSize <= 0 || receivedSize > GetFreeSize())
			return false;

		bufferedSize += receivedSize;
		int offset = 0;
		while (offset < bufferedSize)
		{
			const char packetType = buffer[offset];
			const int packetSize = GetPacketSize(packetType);
			if (packetSize <= 0 || packetSize > kCapacity)
				return false;
			if (packetSize > bufferedSize - offset)
				break;

			if (!handlePacket(packetType, buffer.data() + offset, packetSize))
				return false;
			offset += packetSize;
		}

		bufferedSize -= offset;
		if (bufferedSize > 0)
			std::memmove(buffer.data(), buffer.data() + offset, bufferedSize);
		return true;
	}

private:
	static int GetPacketSize(char packetType)
	{
		switch (packetType)
		{
		case CS_KeyInfo: return sizeof(PlayerInputPacket);
		case CS_PingpongInfo: return sizeof(PingpongPacket);
		default: return 0;
		}
	}

	static_assert(sizeof(PlayerInputPacket) <= kCapacity, "Input packet exceeds receive capacity");
	static_assert(sizeof(PingpongPacket) <= kCapacity, "Ping packet exceeds receive capacity");
	std::array<char, kCapacity> buffer{};
	int bufferedSize = 0;
};
