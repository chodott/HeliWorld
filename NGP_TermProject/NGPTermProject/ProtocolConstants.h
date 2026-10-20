#pragma once
#include <cstdint>

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

	// 예측 기록은 서버 수신 여부와 관계없이 최신 로컬 예측 틱부터 50틱(약 1.67초)을 보관한다.
	// 양 끝 틱을 포함하므로 최대 51개이며, 서버의 200ms 입력 허용 시간과는 별도 기준이다.
	constexpr uint64_t kClientPredictionHistoryTicks = 50;
	constexpr uint64_t kClientPredictionHistoryCapacity = kClientPredictionHistoryTicks + 1;
	// 원격 오브젝트 보간용 서버 스냅샷의 보관 범위는 서버 수신 틱을 기준으로 계산한다.
	constexpr uint64_t kServerSnapshotHistoryTicks = 50;
	constexpr uint64_t kServerSnapshotHistoryCapacity = kServerSnapshotHistoryTicks + 1;

	constexpr uint64_t kMaxRollbackTicks = 100;
}
