#pragma once

namespace Protocol
{
	constexpr double kFixedTick = 1.0 / 30.0;
	constexpr double kFixedTickMs = kFixedTick * 1000.0;
	constexpr float kMsToSec = 0.001f;
	constexpr float kDistanceMargin = 1.2f;

	constexpr int kMaxPlayerCount = 4;
	constexpr float kSpeedPlayerPerSec = 50.0f;

	constexpr int kMaxItemCount = 10;

	constexpr int kMaxMissileCountPerPlayer = 8;

	constexpr int kMaxMissileCount = kMaxPlayerCount * kMaxMissileCountPerPlayer;

	constexpr int kDefaultDelayMs = 150;

	constexpr uint64_t kMaxRollbackTicks = 100;
}
