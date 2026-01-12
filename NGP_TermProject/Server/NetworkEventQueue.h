#pragma once

#include <concurrent_queue.h>

class NetworkEventQueue
{
public:
	template <typename T>
	inline void PushPacket(const T& packet) { GetQueue<T>().push(packet); }
	template <typename T>
	inline bool TryPopPacket(T& outPacket) { return GetQueue<T>().try_pop(outPacket); }

private:
	template <typename T>
	static concurrency::concurrent_queue<T>& GetQueue()
	{
		static concurrency::concurrent_queue<T> queue;
		return queue;
	}

};

