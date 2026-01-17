#pragma once

#include "CSPacket.h"

class IPacketListener 
{
public:
	virtual void OnReceivePacket(char packetType, const char* buffer) = 0;
};