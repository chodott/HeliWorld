#pragma once

namespace Protocol
{
	constexpr double kFixedTick = 1.0 / 30.0;
	constexpr double kFixedTickMs = kFixedTick * 1000.0;

	constexpr int kMaxPlayerCount = 4;

	constexpr int kMaxItemCount = 10;

	constexpr int kMaxMissileCountPerPlayer = 8;

	constexpr int kMaxMissileCount = kMaxPlayerCount * kMaxMissileCountPerPlayer;

	constexpr int kDefaultDelayMs = 150;

}
