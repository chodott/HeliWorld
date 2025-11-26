#pragma once

#include "SCPacket.h"

struct PlayerSnapshot
{
    XMFLOAT3 position;
    XMFLOAT3 rotation;
    int hp;
};

struct MissileSnapshot
{
    XMFLOAT3 position;
    XMFLOAT3 rotation;
    float lifeTime;
    bool active;
};

struct ItemSnapshot
{
    XMFLOAT3 position;
    float lifeTime;
};

struct ServerSnapshot 
{
    PlayerSnapshot playerSnapshots[4];
    ItemSnapshot itemSnapshots[10];
    MissileSnapshot missileSnapshots[32];
};

