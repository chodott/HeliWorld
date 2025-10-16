#pragma once

#include "SCPacket.h"

struct ServerSnapshot {
    PlayerInfoPacket playerInfos[4];
    ItemInfoPacket itemInfos[10];
    MissileInfoPacket missileInfos[32];
};