#pragma once
#include <cstdint>

namespace Protocol
{
	constexpr int kTickRate = 30;
	constexpr double kFixedTick = 1.0 / kTickRate;
	constexpr double kFixedTickMs = kFixedTick * 1000.0;
	constexpr float kMsToSec = 0.001f;
	constexpr float kDistanceMargin = 1.2f;

	constexpr int kMaxPlayerCount = 4;
	constexpr float kSpeedPlayerPerSec = 50.0f;

	constexpr int kMaxItemCount = 10;

	constexpr int kMaxMissileCountPerPlayer = 8;

	constexpr int kMaxMissileCount = kMaxPlayerCount * kMaxMissileCountPerPlayer;

	constexpr int kDefaultDelayMs = 150;

	// 지연 입력은 최근 200ms에 해당하는 완료 틱만 수정할 수 있다.
	constexpr int kMaxLateInputMs = 200;
	// 허용 시간을 틱 단위로 올림한다. 30Hz에서는 최근 완료된 입력 6틱이다.
	constexpr int kRollbackInputTicks = (kMaxLateInputMs * kTickRate + 999) / 1000;
	// 첫 재실행 입력 직전 상태가 필요하므로 완료 입력 6틱에 상태 1개를 더 보관한다.
	constexpr int kSnapshotHistoryCapacity = kRollbackInputTicks + 1;
	// 재실행 준비 중에는 과거 입력 6틱과 이번에 실행할 새 입력 1틱이 동시에 필요하다.
	constexpr int kInputHistoryCapacity = kRollbackInputTicks + 1;
	// 시각 추정 오차로 앞서 도착한 입력은 최대 2틱까지 대기시킨다.
	constexpr int kMaxFutureInputTicks = 2;
	constexpr int kPendingInputCapacity = kRollbackInputTicks + kMaxFutureInputTicks;

	// 기준은 마지막 완료 틱 T이다. 입력 0은 초기 상태이므로 처리하지 않는다.
	constexpr uint64_t GetOldestInputTick(uint64_t lastCompletedTick)
	{
		return lastCompletedTick >= kRollbackInputTicks
			? lastCompletedTick - kRollbackInputTicks + 1 : 1;
	}

	constexpr bool IsInputTickAllowed(uint64_t tick, uint64_t lastCompletedTick)
	{
		return tick >= GetOldestInputTick(lastCompletedTick)
			&& (tick <= lastCompletedTick || tick - lastCompletedTick <= kMaxFutureInputTicks);
	}
}
