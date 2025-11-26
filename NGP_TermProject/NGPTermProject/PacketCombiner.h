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

	template<typename PacketType>
	void CombinePacket(const PacketType& packet);

    template<>
    void CombinePacket<ItemInfoBundlePacket>(const ItemInfoBundlePacket& pkt)
    {
        memcpy(currentFrameData.itemInfos, pkt.itemInfos, sizeof(ItemInfoPacket) * 10);
        frameDataManager->AddServerFrameData(currentFrameData);
    }

    template<>
    void CombinePacket<MissileInfoBundlePacket>(const MissileInfoBundlePacket& pkt)
    {
        memcpy(currentFrameData.missileInfos, pkt.missileInfos, sizeof(MissileInfoPacket) * 32);
    }

    template<>
    void CombinePacket<PlayerInfoBundlePacket>(const PlayerInfoBundlePacket& pkt)
    {
        memcpy(currentFrameData.playerInfos, pkt.playerInfos, sizeof(PlayerInfoPacket) * 4);
        currentFrameData.serverTick = pkt.serverTick;
    }


};