#pragma once

class NetworkServer;
class SimulationServer;
class Client;
class ClientInputBuffer;
class SnapshotPacketBuffer;

struct ServerContext {
	NetworkServer* netServer;   
	SimulationServer* simServer; 
	array<ClientInputBuffer, Protocol::kMaxPlayerCount>* inputBuffer;
	SnapshotPacketBuffer* snapshotBuffer;
};

struct ReceiveClientContext {
	ServerContext* serverContext;
	Client* client;
};
