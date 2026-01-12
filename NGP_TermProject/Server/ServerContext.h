#pragma once

class NetworkServer;
class SimulationServer;
class Client;
class ClientInputBuffer;

struct ServerContext {
	NetworkServer* netServer;   
	SimulationServer* simServer; 
	ClientInputBuffer* inputBuffer;
};

struct ReceiveClientContext {
	ServerContext* serverContext;
	Client* client;
};
