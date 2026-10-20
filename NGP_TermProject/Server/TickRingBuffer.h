#pragma once

#include <array>
#include <cstddef>
#include <cstdint>

// 틱을 인덱스로 사용하는 고정 크기 기록. 오래된 칸은 순환하며 덮어쓴다.
template<typename T, std::size_t Capacity>
class TickRingBuffer
{
	static_assert(Capacity > 0, "Tick history requires at least one slot");
	struct Slot
	{
		uint64_t tick = 0;
		bool valid = false;
		T value{};
	};
	std::array<Slot, Capacity> slots{};

public:
	T& Store(uint64_t tick, const T& value)
	{
		auto& slot = slots[tick % Capacity];
		slot.value = value;
		slot.tick = tick;
		slot.valid = true;
		return slot.value;
	}

	const T* Find(uint64_t tick) const
	{
		const auto& slot = slots[tick % Capacity];
		// 같은 칸이 다른 틱으로 덮어써졌다면 만료된 기록이다.
		return slot.valid && slot.tick == tick ? &slot.value : nullptr;
	}

	void Erase(uint64_t tick)
	{
		auto& slot = slots[tick % Capacity];
		if (slot.valid && slot.tick == tick)
			slot.valid = false;
	}

	void DiscardBefore(uint64_t oldestTick)
	{
		for (auto& slot : slots)
			if (slot.valid && slot.tick < oldestTick)
				slot.valid = false;
	}

	void Clear()
	{
		for (auto& slot : slots)
			slot.valid = false;
	}
};
