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
	void CombinePacket(const PacketType& packet, uint64_t cutTimeline = 0);

    template<>
    void CombinePacket<ItemInfoBundlePacket>(const ItemInfoBundlePacket& pkt, uint64_t cutTimeline)
    {
        memcpy(currentFrameData.itemInfos, pkt.itemInfos, sizeof(ItemInfoPacket) * 10);
        frameDataManager->AddServerFrameData(currentFrameData, cutTimeline);
    }

    template<>
    void CombinePacket<MissileInfoBundlePacket>(const MissileInfoBundlePacket& pkt, uint64_t cutTimeline)
    {
        memcpy(currentFrameData.missileInfos, pkt.missileInfos, sizeof(MissileInfoPacket) * 32);
    }

    template<>
    void CombinePacket<PlayerInfoBundlePacket>(const PlayerInfoBundlePacket& pkt, uint64_t cutTimeline)
    {
        memcpy(currentFrameData.playerInfos, pkt.playerInfos, sizeof(PlayerInfoPacket) * 4);
        currentFrameData.timestamp = pkt.timestamp;
    }


};