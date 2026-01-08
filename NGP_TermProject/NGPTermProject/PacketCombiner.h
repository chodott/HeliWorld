#pragma once
#include "CSPacket.h"
#include "FrameData.h"
#include "FrameDataManager.h"
class PacketCombiner
{
private:
    FrameDataManager* frameDataManager;
    ServerFrameData currentFrameData;

public:
    PacketCombiner();
    PacketCombiner(FrameDataManager* frameDataManager)
    {
        this->frameDataManager = frameDataManager;
    }

};