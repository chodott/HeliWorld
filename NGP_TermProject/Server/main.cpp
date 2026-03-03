#include "NetworkServer.h"

int main()
{
	SnapshotPacketBuffer snapshotPacketBuffer;
	NetworkEventQueue networkEventQueue;
	InputManager inputManager;

	SimulationServer simulationServer(inputManager, snapshotPacketBuffer, networkEventQueue);
	NetworkServer networkServer(snapshotPacketBuffer, networkEventQueue);

	ServerContext serverContext{&networkServer,&simulationServer, &inputManager, &snapshotPacketBuffer};

	networkServer.OpenListenSocket(&serverContext);

	auto  prev = std::chrono::steady_clock::now();
	double acc = 0.0;
	bool isRunning = true;

	while (isRunning)
	{
		//Tick Base
		auto now = std::chrono::steady_clock::now();
		double frameDelta = std::chrono::duration<double>(now - prev).count();
		prev = now;
		acc += frameDelta;

		int steps = 0, maxSteps = 6;
		while (acc >= Protocol::kFixedTick && steps < maxSteps)
		{
			simulationServer.Update(Protocol::kFixedTick);
			acc -= Protocol::kFixedTick;
			++steps;
		}

		if (acc < Protocol::kFixedTick)
		{
			double margin = Protocol::kFixedTick - acc;
			DWORD sleepMs = (DWORD)(margin * 1000.0);
			Sleep(sleepMs);
		}
	}

	networkServer.CloseListenSocket();
}