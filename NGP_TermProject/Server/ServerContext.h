#pragma once

class NetworkServer;
class SimulationServer;
class Client;
class ClientInputBuffer;
class SnapshotPacketBuffer;
class InputManager;

struct ServerContext {
	NetworkServer* netServer;   
	SimulationServer* simServer; 
	InputManager* inputManager;
	SnapshotPacketBuffer* snapshotBuffer;
};

struct ReceiveClientContext {
	ServerContext* serverContext;
	Client* client;
};
