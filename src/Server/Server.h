#pragma once
#include <stdint.h>
#include <vector>

#include "Network/Sockets.h" 

#define BUFSIZE 16384
#define MAXCONNECTIONS 20

enum e_state{
	STATE_FREE,
	STATE_NEW,
	STATE_CONNECTED,
	STATE_DISCONNECTED
};

class Server {
public:
	Server() = default;
	~Server();
	void HandleClient(Socket clientSocket);
	void Run(int port);


private:
	Socket* m_listenSocket = new Socket();
	WSAPOLLFD* m_connectionStates;
	int m_fdCount = 0;
	int m_fdSize = 20;

	void InitClientStates(int start, int end);
};

